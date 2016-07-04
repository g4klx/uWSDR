/*
 *   Copyright (C) 2007,2013 by Jonathan Naylor G4KLX
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

#include "JackReaderWriter.h"

#if defined(USE_JACK)

int jcrwCallback(jack_nframes_t nFrames, void* userData)
{
	wxASSERT(userData != NULL);

	CJackReaderWriter* object = reinterpret_cast<CJackReaderWriter*>(userData);

	return object->callback(nFrames);
}


CJackReaderWriter::CJackReaderWriter(const wxString& name, unsigned int inChannels, unsigned int outChannels) :
m_name(name),
m_inChannels(inChannels),
m_outChannels(outChannels),
m_blockSize(0U),
m_callback(NULL),
m_id(0),
m_client(NULL),
m_inPortI(NULL),
m_inPortQ(NULL),
m_outPortI(NULL),
m_outPortQ(NULL),
m_buffer(NULL),
m_lastBuffer(NULL),
m_inBuffer(NULL),
m_outBuffer(NULL),
m_requests(0),
m_underruns(0),
m_overruns(0),
m_enabled(false),
m_opened(0),
m_active(false)
{
	wxASSERT(inChannels <= 2U);
	wxASSERT(outChannels <= 2U);
	wxASSERT(inChannels > 0U || outChannels > 0U);
}

CJackReaderWriter::~CJackReaderWriter()
{
	delete   m_buffer;
	delete[] m_lastBuffer;
	delete[] m_inBuffer;
	delete[] m_outBuffer;
}

void CJackReaderWriter::setCallback(IDataCallback* callback, int id)
{
	wxASSERT(callback != NULL);

	m_callback = callback;
	m_id       = id;
}

bool CJackReaderWriter::open(float sampleRate, unsigned int blockSize)
{
	if (m_opened > 0U) {
		m_opened++;
		return true;
	}

	m_blockSize = blockSize;
	m_enabled   = false;

	jack_status_t status;
	m_client = ::jack_client_open(m_name.char_str(), JackNullOption, &status, NULL);
	if (m_client == NULL) {
		if (status & JackServerFailed)
			wxLogError(wxT("JackReaderWriter: unabled to start the Jack server"));
		else
			wxLogError(wxT("JackReaderWriter: received 0x%02X from jack_client_open()"), status);
		return false;
	}

	if (status & JackNameNotUnique) {
		wxLogError(wxT("JackReaderWriter: client '%s' name is not unique"), m_name.c_str());
		::jack_client_close(m_client);
		return false;
	}

	int ret = ::jack_set_process_callback(m_client, jcrwCallback, this);
	if (ret != 0) {
		wxLogError(wxT("JackReaderWriter: unable to set the callback, error = %d"), ret);
		::jack_client_close(m_client);
		return false;
	}

	jack_nframes_t jsr = ::jack_get_sample_rate(m_client);
	if (float(jsr) != sampleRate) {
		wxLogError(wxT("JackReaderWriter: incorrect Jack sample rate of %u"), jsr);
		::jack_client_close(m_client);
		return false;
	}

	switch (m_inChannels) {
		case 0U:
			break;
		case 1U:
			m_inPortI = ::jack_port_register(m_client, "input", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

			if (m_inPortI == NULL) {
				wxLogError(wxT("JackReaderWriter: unable to open the Jack input port"));
				::jack_client_close(m_client);
				return false;
			}
			break;
		case 2U:
			m_inPortI = ::jack_port_register(m_client, "input_i", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

			if (m_inPortI == NULL) {
				wxLogError(wxT("JackReaderWriter: unable to open the Jack I input port"));
				::jack_client_close(m_client);
				return false;
			}

			m_inPortQ = ::jack_port_register(m_client, "input_q", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

			if (m_inPortQ == NULL) {
				wxLogError(wxT("JackReaderWriter: unable to open the Jack Q input port"));
				::jack_client_close(m_client);
				return false;
			}
			break;
	}

	if (m_inChannels > 0U)
		m_inBuffer  = new float[blockSize * 2U];

	switch (m_outChannels) {
		case 0U:
			break;
		case 1U:
			m_outPortI = ::jack_port_register(m_client, "output", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

			if (m_outPortI == NULL) {
				wxLogError(wxT("JackReaderWriter: unable to open the Jack output port"));
				::jack_client_close(m_client);
				return false;
			}
			break;
		case 2U:
			m_outPortI = ::jack_port_register(m_client, "output_i", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

			if (m_outPortI == NULL) {
				wxLogError(wxT("JackReaderWriter: unable to open the Jack I output port"));
				::jack_client_close(m_client);
				return false;
			}

			m_outPortQ = ::jack_port_register(m_client, "output_q", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

			if (m_outPortQ == NULL) {
				wxLogError(wxT("JackReaderWriter: unable to open the Jack Q output port"));
				::jack_client_close(m_client);
				return false;
			}
			break;
	}

	if (m_outChannels > 0U) {
		m_outBuffer = new float[blockSize * 2U];
		m_buffer    = new CRingBuffer(blockSize * 5U, 2U);

		m_lastBuffer = new float[blockSize * 2U];
		::memset(m_lastBuffer, 0x00, blockSize * 2U * sizeof(float));
	}

	ret = ::jack_activate(m_client);
	if (ret != 0) {
		wxLogError(wxT("JackReaderWriter: error %d when activating the Jack client"), ret);
		::jack_client_close(m_client);
		return false;
	}

	m_opened++;
	m_active = true;

	wxLogMessage(wxT("JackReaderWriter: started for client %s"), m_name.c_str());

	return true;
}

void CJackReaderWriter::write(const float* buffer, unsigned int nSamples)
{
	if (!m_enabled || m_outChannels == 0U)
		return;

	if (nSamples == 0U)
		return;

	wxASSERT(buffer != NULL);

	unsigned int n = m_buffer->addData(buffer, nSamples);

	if (n != nSamples)
		m_overruns++;
}

int CJackReaderWriter::callback(jack_nframes_t nFrames)
{
	m_requests++;

	if (nFrames != m_blockSize) {
		wxLogError(wxT("JackReaderWriter: invalid frame size, given %u, wanted %u"), nFrames, m_blockSize);
		nFrames = m_blockSize;
	}

	switch (m_inChannels) {
		case 0U:
			break;

		case 1U: {
				wxASSERT(m_callback != NULL);
				wxASSERT(m_inBuffer != NULL);
				wxASSERT(m_inPortI != NULL);

				jack_default_audio_sample_t* input = (jack_default_audio_sample_t*)::jack_port_get_buffer(m_inPortI, nFrames);

				wxASSERT(input != NULL);

				for (unsigned int i = 0U; i < nFrames; i++) {
					m_inBuffer[i * 2U + 0U] = input[i];
					m_inBuffer[i * 2U + 1U] = input[i];
				}

				m_callback->callback(m_inBuffer, nFrames, m_id);
			}
			break;			

		case 2U: {
				wxASSERT(m_callback != NULL);
				wxASSERT(m_inBuffer != NULL);
				wxASSERT(m_inPortI != NULL);
				wxASSERT(m_inPortQ != NULL);

				jack_default_audio_sample_t* inputI = (jack_default_audio_sample_t*)::jack_port_get_buffer(m_inPortI, nFrames);
				jack_default_audio_sample_t* inputQ = (jack_default_audio_sample_t*)::jack_port_get_buffer(m_inPortQ, nFrames);

				wxASSERT(inputI != NULL);
				wxASSERT(inputQ != NULL);

				for (unsigned int i = 0U; i < nFrames; i++) {
					m_inBuffer[i * 2U + 0U] = inputI[i];
					m_inBuffer[i * 2U + 1U] = inputQ[i];
				}

				m_callback->callback(m_inBuffer, nFrames, m_id);
			}
			break;			
	}

	switch (m_outChannels) {
		case 0U:
			break;

		case 1U: {
				wxASSERT(m_buffer != NULL);
				wxASSERT(m_outBuffer != NULL);
				wxASSERT(m_lastBuffer != NULL);
				wxASSERT(m_outPortI != NULL);

				jack_default_audio_sample_t* output = (jack_default_audio_sample_t*)::jack_port_get_buffer(m_outPortI, nFrames);

				wxASSERT(output != NULL);

				if (m_buffer->dataSpace() >= nFrames) {
					m_buffer->getData(m_outBuffer, nFrames);
				} else {
					::memcpy(m_outBuffer, m_lastBuffer, nFrames * 2U * sizeof(float));
					m_underruns++;
				}

				::memcpy(m_lastBuffer, m_outBuffer, nFrames * 2U * sizeof(float));

				for (unsigned int i = 0U; i < nFrames; i++)
					output[i] = m_outBuffer[i * 2U + 0U];
			}
			break;

		case 2U: {
				wxASSERT(m_buffer != NULL);
				wxASSERT(m_outBuffer != NULL);
				wxASSERT(m_lastBuffer != NULL);
				wxASSERT(m_outPortI != NULL);
				wxASSERT(m_outPortQ != NULL);

				jack_default_audio_sample_t* outputI = (jack_default_audio_sample_t*)::jack_port_get_buffer(m_outPortI, nFrames);
				jack_default_audio_sample_t* outputQ = (jack_default_audio_sample_t*)::jack_port_get_buffer(m_outPortQ, nFrames);

				wxASSERT(outputI != NULL);
				wxASSERT(outputQ != NULL);

				if (m_buffer->dataSpace() >= nFrames) {
					m_buffer->getData(m_outBuffer, nFrames);
				} else {
					::memcpy(m_outBuffer, m_lastBuffer, nFrames * 2U * sizeof(float));
					m_underruns++;
				}

				::memcpy(m_lastBuffer, m_outBuffer, nFrames * 2U * sizeof(float));

				for (unsigned int i = 0U; i < nFrames; i++) {
					outputI[i] = m_outBuffer[i * 2U + 0U];
					outputQ[i] = m_outBuffer[i * 2U + 1U];
				}
			}
			break;
	}

	return 0;
}

/*
 * Close the sound card on the first call to close() all later ones are NOPs until the last one which also
 * delete's the object.
 */
void CJackReaderWriter::close()
{
	if (!m_active) {
		if (m_opened == 0U) {
			delete this;
			return;
		}

		m_opened--;
		return;
	}

	m_opened--;
	m_active = false;

	int ret = ::jack_client_close(m_client);
	if (ret != 0)
		wxLogError(wxT("JackReaderWriter: received %d from jack_client_close()"), ret);

	wxLogMessage(wxT("JackReaderWriter: %u underruns and %u overruns from %u requests"), m_underruns, m_overruns, m_requests);
}

void CJackReaderWriter::enable(bool enable)
{
	if (m_outChannels == 0U)
		return;

	m_enabled = enable;

	if (!enable)
		::memset(m_lastBuffer, 0x00, m_blockSize * 2U * sizeof(float));
}

void CJackReaderWriter::disable()
{
	enable(false);
}

void CJackReaderWriter::purge()
{
}

bool CJackReaderWriter::hasClock()
{
	return true;
}

void CJackReaderWriter::clock()
{
}

#else

CJackReaderWriter::CJackReaderWriter(const wxString& WXUNUSED(name), unsigned int WXUNUSED(inChannels), unsigned int WXUNUSED(outChannels))
{
}

CJackReaderWriter::~CJackReaderWriter()
{
}

void CJackReaderWriter::setCallback(IDataCallback* WXUNUSED(callback), int WXUNUSED(id))
{
}

bool CJackReaderWriter::open(float WXUNUSED(sampleRate), unsigned int WXUNUSED(blockSize))
{
	wxLogError(wxT("JackReaderWriter: UWSDR has been built without Jack support"));		

	return false;
}

void CJackReaderWriter::write(const float* WXUNUSED(buffer), unsigned int WXUNUSED(nSamples))
{
}

void CJackReaderWriter::close()
{
}

void CJackReaderWriter::enable(bool WXUNUSED(enable))
{
}

void CJackReaderWriter::disable()
{
}

void CJackReaderWriter::purge()
{
}

bool CJackReaderWriter::hasClock()
{
	return false;
}

void CJackReaderWriter::clock()
{
}

#endif
