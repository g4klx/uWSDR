/*
 *   Copyright (C) 2006-2008,2013 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "DSPControl.h"
#include "UWSDRApp.h"

#include <wx/datetime.h>


const int TX_READER       = 44;
const int RX_READER       = 55;
const int CW_READER       = 66;
const int VOICE_READER    = 77;
const int EXTERNAL_READER = 88;

const unsigned int RINGBUFFER_SIZE = 100001;

CDSPControl::CDSPControl(float sampleRate, unsigned int receiveGainOffset, unsigned int blockSize, bool swapIQ) :
wxThread(),
m_dttsp(NULL),
m_cwKeyer(NULL),
m_voiceKeyer(NULL),
m_sampleRate(sampleRate),
m_blockSize(blockSize),
m_txReader(NULL),
m_txWriter(NULL),
m_rxReader(NULL),
m_rxWriter(NULL),
m_txInControl(NULL),
m_keyInControl(NULL),
m_waiting(),
m_txRingBuffer(RINGBUFFER_SIZE, 2),
m_rxRingBuffer(RINGBUFFER_SIZE, 2),
m_txBuffer(NULL),
m_rxBuffer(NULL),
m_outBuffer(NULL),
m_txLastBuffer(NULL),
m_rxLastBuffer(NULL),
m_record(NULL),
m_transmit(TXSTATE_RX),
m_running(false),
m_mode(MODE_USB),
m_recordType(RECORD_MONO_AUDIO),
m_afGain(0.0F),
m_clockId(-1),
m_lastTXIn(false),
m_lastKeyIn(false),
m_rxOverruns(0U),
m_rxFills(0U),
m_txOverruns(0U),
m_txFills(0U),
m_extHandler(NULL)
{
	m_dttsp = new CDTTSPControl();
	m_dttsp->open(m_sampleRate, receiveGainOffset, m_blockSize, swapIQ);

	m_cwKeyer = new CCWKeyer();
	m_cwKeyer->setCallback(this, CW_READER);
	m_cwKeyer->open(m_sampleRate, m_blockSize);

	m_voiceKeyer = new CVoiceKeyer();
	m_voiceKeyer->setCallback(this, VOICE_READER);
	m_voiceKeyer->open(m_sampleRate, m_blockSize);

	m_txBuffer  = new float[m_blockSize * 2];
	m_rxBuffer  = new float[m_blockSize * 2];
	m_outBuffer = new float[m_blockSize * 2];

	m_txLastBuffer = new float[m_blockSize * 2];
	m_rxLastBuffer = new float[m_blockSize * 2];
}

CDSPControl::~CDSPControl()
{
	delete[] m_txBuffer;
	delete[] m_rxBuffer;
	delete[] m_outBuffer;
	delete[] m_txLastBuffer;
	delete[] m_rxLastBuffer;
}

void CDSPControl::setTXReader(IDataReader* reader)
{
	wxASSERT(reader != NULL);
	wxASSERT(m_txReader == NULL);

	m_txReader = reader;
}

void CDSPControl::setTXWriter(IDataWriter* writer)
{
	wxASSERT(writer != NULL);
	wxASSERT(m_txWriter == NULL);

	m_txWriter = writer;
}

void CDSPControl::setRXReader(IDataReader* reader)
{
	wxASSERT(reader != NULL);
	wxASSERT(m_rxReader == NULL);

	m_rxReader = reader;
}

void CDSPControl::setRXWriter(IDataWriter* writer)
{
	wxASSERT(writer != NULL);

	m_rxWriter = writer;
}

void CDSPControl::setTXInControl(IExternalInterface* control)
{
	wxASSERT(control != NULL);

	m_txInControl = control;
}

void CDSPControl::setKeyInControl(IExternalInterface* control)
{
	wxASSERT(control != NULL);

	m_keyInControl = control;
}

bool CDSPControl::open()
{
	bool ret = openIO();
	if (!ret)
		return false;

	Create();
	Run();

	return true;
}

void* CDSPControl::Entry()
{
	// Open for business
	m_running = true;

	m_rxWriter->enable();

	while (!TestDestroy()) {
		wxSemaError ret = m_waiting.WaitTimeout(500UL);

		if (ret == wxSEMA_NO_ERROR) {
			if (m_transmit == TXSTATE_INT || m_transmit == TXSTATE_EXT) {
				unsigned int nSamples = m_txRingBuffer.getData(m_txBuffer, m_blockSize);
				if (nSamples != m_blockSize) {
					// Copy the last buffer of good data to the output
					::memcpy(m_txBuffer, m_txLastBuffer, m_blockSize * sizeof(float) * 2U);
					m_txFills++;
				} else {
					// Save the last full buffer in case it's needed
					::memcpy(m_txLastBuffer, m_txBuffer, m_blockSize * sizeof(float) * 2U);
				}

				if (nSamples > 0U)
					m_txWriter->write(m_txBuffer, nSamples);
			}

			unsigned int nSamples = m_rxRingBuffer.getData(m_rxBuffer, m_blockSize);

			// Create silence on transmit if no sidetone is being transmitted
			if (nSamples == 0U && (m_transmit == TXSTATE_INT || m_transmit == TXSTATE_EXT)) {
				::memset(m_rxBuffer, 0x00U, m_blockSize * 2U * sizeof(float));
				nSamples = m_blockSize;
			}

			// Only send audio to the external protocol handler when not transmitting
			if (m_extHandler != NULL && m_transmit != TXSTATE_EXT)
				m_extHandler->writeAudio(m_rxBuffer, nSamples);

			if (nSamples != m_blockSize) {
				// Copy the last buffer of good data to the output
				::memcpy(m_rxBuffer, m_rxLastBuffer, m_blockSize * sizeof(float) * 2U);
				m_rxFills++;
			} else {
				// Save the last full buffer in case it's needed
				::memcpy(m_rxLastBuffer, m_rxBuffer, m_blockSize * sizeof(float) * 2U);
			}

			// Don't record when transmitting
			if (m_record != NULL && (m_recordType == RECORD_MONO_AUDIO || m_recordType == RECORD_STEREO_AUDIO) && m_transmit == TXSTATE_RX)
				m_record->write(m_rxBuffer, nSamples);

			// Scale the audio by the AF Gain setting
			for (unsigned int i = 0U; i < (nSamples * 2U); i++)
				m_rxBuffer[i] *= m_afGain;

			m_rxWriter->write(m_rxBuffer, nSamples);
		}
	}

	m_running = false;

	m_rxWriter->disable();
	m_txWriter->disable();

	wxLogMessage(wxT("DSPControl: RX Overruns=%u, RX Fills=%u"), m_rxOverruns, m_rxFills);
	wxLogMessage(wxT("DSPControl: TX Overruns=%u, TX Fills=%u"), m_txOverruns, m_txFills);

	closeIO();

	return (void*)0;
}

void CDSPControl::close()
{
	Delete();
}

bool CDSPControl::openIO()
{
	// This should be done before opening
	m_txReader->setCallback(this, TX_READER);
	m_rxReader->setCallback(this, RX_READER);

	bool ret = m_txWriter->open(m_sampleRate, m_blockSize);
	if (!ret)
		return false;

	ret = m_txReader->open(m_sampleRate, m_blockSize);
	if (!ret) {
		m_dttsp->close();
		m_cwKeyer->close();
		m_voiceKeyer->close();

		return false;
	}

	if (m_txReader->hasClock())
		m_clockId = TX_READER;

	ret = m_rxWriter->open(m_sampleRate, m_blockSize);
	if (!ret) {
		m_dttsp->close();
		m_cwKeyer->close();
		m_voiceKeyer->close();

		return false;
	}

	ret = m_rxReader->open(m_sampleRate, m_blockSize);
	if (!ret) {
		m_dttsp->close();
		m_cwKeyer->close();
		m_voiceKeyer->close();

		return false;
	}

	if (m_extHandler != NULL) {
		m_extHandler->setCallback(this, EXTERNAL_READER);

		ret = m_extHandler->open();
		if (!ret) {
			m_dttsp->close();
			m_cwKeyer->close();
			m_voiceKeyer->close();

			return false;
		}
	}

	// Open the TX In port and set the relevant output pin high to be shorted to the relevant input pin
	if (m_txInControl != NULL) {
		ret = m_txInControl->open();

		if (!ret) {
			m_dttsp->close();
			m_cwKeyer->close();
			m_voiceKeyer->close();

			return false;
		}
	}

	// Open the Key In port and set the relevant output pin high to be shorted to the relevant input pin
	if (m_keyInControl != NULL) {
		ret = m_keyInControl->open();

		if (!ret) {
			m_dttsp->close();
			m_cwKeyer->close();
			m_voiceKeyer->close();

			return false;
		}
	}

	if (m_rxReader->hasClock())
		m_clockId = RX_READER;

	if (m_clockId == -1) {
		m_dttsp->close();
		m_cwKeyer->close();
		m_voiceKeyer->close();

		wxLogError(wxT("No reader can provide a suitable clock"));
		return false;
	}

	return true;
}

void CDSPControl::closeIO()
{
	setRecordOff();

	m_rxReader->close();
	m_txReader->close();
	m_rxWriter->close();
	m_txWriter->close();

	if (m_txInControl != NULL)
		m_txInControl->close();

	if (m_keyInControl != NULL)
		m_keyInControl->close();

	if (m_extHandler != NULL)
		m_extHandler->close();

	m_dttsp->close();
	m_cwKeyer->close();
	m_voiceKeyer->close();

	if (m_record != NULL) {
		m_record->disable();
		m_record->close();
	}
}

void CDSPControl::callback(float* inBuffer, unsigned int nSamples, int id)
{
	// Don't process any data until the main thread is ready
	if (!m_running)
		return;

	wxASSERT(inBuffer != NULL);
	wxASSERT(nSamples > 0);

	// Use whatever clock is available to run everything
	if (id == m_clockId) {
		if (m_transmit == TXSTATE_INT || m_transmit == TXSTATE_EXT) {
			if (id != TX_READER)
				m_txReader->clock();

			if (m_cwKeyer->isActive() && (m_mode == MODE_CWUN || m_mode == MODE_CWUW || m_mode == MODE_CWLN || m_mode == MODE_CWLW))
				m_cwKeyer->clock();

			if (m_voiceKeyer->isActive() && m_mode != MODE_CWUN && m_mode != MODE_CWUW && m_mode != MODE_CWLN && m_mode != MODE_CWLW)
				m_voiceKeyer->clock();
		} else {
			if (id != RX_READER)
				m_rxReader->clock();
		}

		if (m_txInControl != NULL)
			m_txInControl->clock();

		if (m_keyInControl != NULL)
			m_keyInControl->clock();

		if (m_extHandler != NULL)
			m_extHandler->clock();
	}

	switch (id) {
		case RX_READER: {
				if (m_transmit != TXSTATE_RX)
					return;

				// Don't record when transmitting
				if (m_record != NULL && m_recordType == RECORD_STEREO_IQ)
					m_record->write(inBuffer, nSamples);

				// Only send data to the external protocol handler when not transmitting
				if (m_extHandler != NULL)
					m_extHandler->writeRaw(inBuffer, nSamples);

				m_dttsp->dataIO(inBuffer, m_outBuffer, nSamples);

				unsigned int n = m_rxRingBuffer.addData(m_outBuffer, nSamples);
				if (n != nSamples)
					m_rxOverruns++;

				if (n > 0U)
					m_waiting.Post();
			}
			break;

		case TX_READER: {
				if (m_transmit != TXSTATE_INT)
					return;

				// If the voice or CW keyer are active, exit
				if (m_voiceKeyer->isActive() && m_mode != MODE_CWUN && m_mode != MODE_CWUW && m_mode != MODE_CWLN && m_mode != MODE_CWLW)
					return;
				if (m_cwKeyer->isActive() && (m_mode == MODE_CWUN || m_mode == MODE_CWUW || m_mode == MODE_CWLN || m_mode == MODE_CWLW))
					return;

				m_dttsp->dataIO(inBuffer, m_outBuffer, nSamples);

				unsigned int n = m_txRingBuffer.addData(m_outBuffer, nSamples);
				if (n != nSamples)
					m_txOverruns++;

				if (n > 0U)
					m_waiting.Post();
			}
			break;

		case VOICE_READER: {
				if (m_transmit != TXSTATE_INT)
					return;

				if (m_mode == MODE_CWUN || m_mode == MODE_CWUW || m_mode == MODE_CWLN || m_mode == MODE_CWLW)
					return;				

				m_dttsp->dataIO(inBuffer, m_outBuffer, nSamples);

				unsigned int n = m_txRingBuffer.addData(m_outBuffer, nSamples);
				if (n != nSamples)
					m_txOverruns++;

				if (n > 0U)
					m_waiting.Post();
			}
			break;

		case CW_READER: {
				if (m_transmit != TXSTATE_INT)
					return;

				if (m_mode != MODE_CWUN && m_mode != MODE_CWUW && m_mode != MODE_CWLN && m_mode != MODE_CWLW)
					return;

				// Send the tone out for the side tone
				unsigned int n1 = m_rxRingBuffer.addData(inBuffer, nSamples);
				if (n1 != nSamples)
					m_rxOverruns++;

				// Now transmit it
				m_dttsp->dataIO(inBuffer, m_outBuffer, nSamples);

				unsigned int n2 = m_txRingBuffer.addData(m_outBuffer, nSamples);
				if (n2 != nSamples)
					m_txOverruns++;

				if (n1 > 0U || n2 > 0U)
					m_waiting.Post();
			}
			break;

		case EXTERNAL_READER: {
				if (m_transmit != TXSTATE_EXT)
					return;

				m_dttsp->dataIO(inBuffer, m_outBuffer, nSamples);

				unsigned int n = m_txRingBuffer.addData(m_outBuffer, nSamples);
				if (n != nSamples)
					m_txOverruns++;

				if (n > 0U)
					m_waiting.Post();
			}
			break;

		default:
			wxLogError(wxT("callback() from unknown source = %d"), id);
			break;
	}

	if (m_txInControl != NULL) {
		bool state = m_txInControl->getState();

		if (state != m_lastTXIn) {
			::wxGetApp().setTransmit(state);
			m_lastTXIn = state;
		}
	}

	// Only service the key input when in CW mode
	if (m_mode == MODE_CWUN || m_mode == MODE_CWUW || m_mode == MODE_CWLN || m_mode == MODE_CWLW) {
		if (m_keyInControl != NULL) {
			bool state = m_keyInControl->getState();

			if (state != m_lastKeyIn) {
				::wxGetApp().setKey(state);
				m_lastKeyIn = state;
			}
		}
	}
}

void CDSPControl::setKey(bool keyOn)
{
	if (keyOn != m_lastKeyIn) {
		m_cwKeyer->key(keyOn);
		m_lastKeyIn = keyOn;
	}
}

void CDSPControl::setMode(UWSDRMODE mode)
{
	// We need a copy too ...
	m_mode = mode;

	m_dttsp->setMode(mode);
}

void CDSPControl::setWeaver(bool onOff)
{
	m_dttsp->setWeaver(onOff);
}

void CDSPControl::setFilter(FILTERWIDTH filter)
{
	m_dttsp->setFilter(filter);
}

void CDSPControl::setAGC(AGCSPEED agc)
{
	m_dttsp->setAGC(agc);
}

void CDSPControl::setDeviation(FMDEVIATION dev)
{
	m_dttsp->setDeviation(dev);
}

void CDSPControl::setTXAndFreq(TXSTATE transmit, float freq)
{
	// On a change from transmit to receive and vice versa we empty the ring buffers and
	// drain the semaphore. We mute the transmit writer when on receive.
	if (transmit != m_transmit) {
		if (transmit == TXSTATE_INT || transmit == TXSTATE_EXT)
			m_txWriter->enable();
		else
			m_txWriter->disable();

		wxSemaError status = m_waiting.TryWait();
		while (status != wxSEMA_BUSY) {
			m_waiting.Wait();
			status = m_waiting.TryWait();
		}

		m_rxRingBuffer.clear();
		m_rxReader->purge();

		m_txRingBuffer.clear();
		m_txReader->purge();

		m_cwKeyer->purge();
		m_voiceKeyer->purge();
	}

	m_transmit = transmit;

	m_dttsp->setTXAndFreq(transmit == TXSTATE_INT || transmit == TXSTATE_EXT, freq);
}

void CDSPControl::setNB(bool onOff)
{
	m_dttsp->setNB(onOff);
}

void CDSPControl::setNBValue(unsigned int value)
{
	m_dttsp->setNBValue(value);
}

void CDSPControl::setNB2(bool onOff)
{
	m_dttsp->setNB2(onOff);
}

void CDSPControl::setNB2Value(unsigned int value)
{
	m_dttsp->setNB2Value(value);
}

void CDSPControl::setSP(bool onOff)
{
	m_dttsp->setSP(onOff);
}

void CDSPControl::setSPValue(unsigned int value)
{
	m_dttsp->setSPValue(value);
}

void CDSPControl::setEQ(bool onOff)
{
	m_dttsp->setEQ(onOff);
}

void CDSPControl::setEQLevels(unsigned int n, const int* levels)
{
	m_dttsp->setEQLevels(n, levels);
}

void CDSPControl::setCarrierLevel(unsigned int value)
{
	m_dttsp->setCarrierLevel(value);
}

void CDSPControl::setALCValue(unsigned int attack, unsigned int decay, unsigned int hang)
{
	m_dttsp->setALCValue(attack, decay, hang);
}

void CDSPControl::setRXIAndQ(int phase, int gain)
{
	m_dttsp->setRXIAndQ(phase, gain);
}

void CDSPControl::setTXIAndQ(int phase, int gain)
{
	m_dttsp->setTXIAndQ(phase, gain);
}

void CDSPControl::setBinaural(bool onOff)
{
	m_dttsp->setBinaural(onOff);
}

void CDSPControl::setPan(int value)
{
	m_dttsp->setPan(value);
}

float CDSPControl::getMeter(METERPOS type)
{
	return m_dttsp->getMeter(type);
}

float CDSPControl::getTXOffset()
{
	return m_dttsp->getTXOffset();
}

float CDSPControl::getRXOffset()
{
	return m_dttsp->getRXOffset();
}

void CDSPControl::getSpectrum(float* spectrum)
{
	m_dttsp->getSpectrum(spectrum);
}

void CDSPControl::getScope(float* spectrum)
{
	m_dttsp->getScope(spectrum);
}

void CDSPControl::getPhase(float* spectrum)
{
	m_dttsp->getPhase(spectrum);
}

void CDSPControl::setAFGain(unsigned int value)
{
	// Map 0 - 1000 to 0.0 - 1.0
	m_afGain = float(value) / 1000.0F;
}

void CDSPControl::setRFGain(unsigned int value)
{
	m_dttsp->setRFGain(value);
}

void CDSPControl::setMicGain(unsigned int value)
{
	m_dttsp->setMicGain(value);
}

void CDSPControl::setPower(unsigned int value)
{
	m_dttsp->setPower(value);
}

void CDSPControl::setSquelch(unsigned int value)
{
	m_dttsp->setSquelch(value);
}

bool CDSPControl::setRecordOn(RECORDTYPE type)
{
	if (m_record == NULL) {
		wxDateTime now = wxDateTime::Now();
		wxString fileName = now.Format(wxT("%Y%m%d-%H%M%S.wav"));

		CSoundFileWriter* sdfw = NULL;
		switch (type) {
			case RECORD_MONO_AUDIO:
				sdfw = new CSoundFileWriter(fileName, 1U, 16U);
				break;
			case RECORD_STEREO_AUDIO:
				sdfw = new CSoundFileWriter(fileName, 2U, 16U);
				break;
			case RECORD_STEREO_IQ:
				sdfw = new CSoundFileWriter(fileName, 2U, 32U);
				break;
			default:
				wxLogError(wxT("Unknown recording type = %d"), type);				
				return false;
		}

		bool ret = sdfw->open(m_sampleRate, m_blockSize);
		if (!ret) {
			wxLogError(wxT("Cannot open file %s for recording"), fileName.c_str());
			return false;
		}

		sdfw->enable();

		m_recordType = type;
		m_record     = sdfw;

		wxLogMessage(wxT("Opened file %s for recording"), fileName.c_str());
	}

	return true;
}

void CDSPControl::setRecordOff()
{
	if (m_record != NULL) {
		m_record->disable();

		CSoundFileWriter* sdfw = m_record;
		m_record = NULL;

		sdfw->close();

		wxLogMessage(wxT("Closed sound file"));
	}
}

CWERROR CDSPControl::sendCW(unsigned int speed, const wxString& text, CWSTATUS state)
{
	return m_cwKeyer->send(speed, text, state);
}

VOICEERROR CDSPControl::sendAudio(const wxString& fileName, VOICESTATUS state)
{
	return m_voiceKeyer->send(fileName, state);
}

void CDSPControl::setExternalHandler(CExternalProtocolHandler* handler)
{
	wxASSERT(handler != NULL);

	m_extHandler = handler;
}

#if defined(__WXDEBUG__)
void CDSPControl::dumpBuffer(const wxString& title, float* buffer, unsigned int nSamples) const
{
	wxASSERT(buffer != NULL);
	wxASSERT(nSamples > 0);

	wxLogMessage(title);
	wxLogMessage(wxT("Length: %05X"), nSamples);

	wxLogMessage(wxT(":"));

	unsigned int n = 0;
	for (unsigned int i = 0; i < nSamples; i += 16) {
		wxString text;
		text.Printf(wxT("%05X:  "), i);

		for (unsigned int j = 0; j < 16; j++, n += 2) {
			wxString buf;
			buf.Printf(wxT("%f:%f "), buffer[n + 0], buffer[n + 1]);
			text.Append(buf);

			if ((i + j) >= nSamples)
				break;
		}

		wxLogMessage(text);
	}
}
#endif
