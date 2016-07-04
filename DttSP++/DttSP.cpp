/* DttSP.cpp

common defs and code for parm update 
   
This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006-5 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2006-2008,2013 by Jonathan Naylor, G4KLX

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

The authors can be reached by email at

ab2kt@arrl.net
or
rwmcgwier@comcast.net

or by paper mail at

The DTTS Microwave Society
6 Kathleen Place
Bridgewater, NJ 08807
*/

#include "DttSP.h"
#include "Defs.h"

CDttSP::CDttSP(float sampleRate, unsigned int audioSize, bool swapIQ) :
m_sampleRate(sampleRate),
m_running(true),
m_suspend(false),
m_update(NULL),
m_buffer(NULL),
m_rx(NULL),
m_tx(NULL),
m_meter(NULL),
m_spectrum(NULL),
m_inputI(NULL),
m_inputQ(NULL),
m_outputI(NULL),
m_outputQ(NULL),
m_lastOutput(NULL),
m_bufferI(NULL),
m_bufferQ(NULL),
m_trx(RX),
m_trxNext(RX),
m_state(RUN_PLAY),
m_stateLast(RUN_PLAY),
m_want(0U),
m_have(0U),
m_fade(0U),
m_tail(0U),
m_frames(audioSize)
{
	wxASSERT(sampleRate > 0.0F);

	m_update = new wxSemaphore();
	m_buffer = new wxSemaphore();

	m_meter    = new CMeter();
	m_spectrum = new CSpectrum(DEFSPEC, FFTW_ESTIMATE, SPEC_PWR);

	m_bufferI = new float[m_frames];
	m_bufferQ = new float[m_frames];
	::memset(m_bufferI, 0x00, m_frames * sizeof(float));
	::memset(m_bufferQ, 0x00, m_frames * sizeof(float));

	m_inputI  = new CRingBuffer(RINGSIZE, 1U);
	m_inputQ  = new CRingBuffer(RINGSIZE, 1U);
	m_outputI = new CRingBuffer(RINGSIZE, 1U);
	m_outputQ = new CRingBuffer(RINGSIZE, 1U);

	m_lastOutput = new float[m_frames * 2U];
	::memset(m_lastOutput, 0x00, m_frames * 2U * sizeof(float));

	m_fade = m_frames / 5U;
	m_tail = m_frames - m_fade;

	m_rx = new CRX(m_frames, FFTW_ESTIMATE, sampleRate, m_meter, m_spectrum, swapIQ);
	m_tx = new CTX(m_frames, FFTW_ESTIMATE, sampleRate, m_meter, m_spectrum, swapIQ);

	m_rx->setMode(DEFMODE);
	m_tx->setMode(DEFMODE);
}

CDttSP::~CDttSP()
{
	m_running = false;
	m_suspend = true;

	delete[] m_bufferI;
	delete[] m_bufferQ;

	delete m_update;
	delete m_buffer;

	delete m_rx;
	delete m_tx;
	delete m_meter;
	delete m_spectrum;
	delete m_inputI;
	delete m_inputQ;
	delete m_outputI;
	delete m_outputQ;;

	delete[] m_lastOutput;
}

void CDttSP::setMode(SDRMODE m)
{
	m_update->Wait();

	m_tx->setMode(m);
	m_rx->setMode(m);

	m_update->Post();
}

void CDttSP::setWeaver(bool flag)
{
	m_tx->setWeaver(flag);
	m_rx->setWeaver(flag);
}

void CDttSP::setDCBlockFlag(bool flag)
{
	m_tx->setDCBlockFlag(flag);
}

void CDttSP::setRXFilter(double lowFreq, double highFreq)
{
	m_update->Wait();

	m_rx->setFilter(lowFreq, highFreq);

	m_update->Post();
}

void CDttSP::setTXFilter(double lowFreq, double highFreq)
{
	m_update->Wait();

	m_tx->setFilter(lowFreq, highFreq);

	m_update->Post();
}

void CDttSP::releaseUpdate()
{
	m_update->Post();
}

void CDttSP::setRXFrequency(double freq)
{
	if (::fabs(freq) >= 0.5 * m_sampleRate)
		return;

	m_rx->setFrequency(freq);
}

void CDttSP::setTXFrequency(double freq)
{
	if (::fabs(freq) >= 0.5 * m_sampleRate)
		return;

	m_tx->setFrequency(freq);
}

void CDttSP::setANRFlag(bool flag)
{
	m_rx->setANRFlag(flag);
}

void CDttSP::setBANRFlag(bool flag)
{
	m_rx->setBANRFlag(flag);
}

void CDttSP::setANRValues(unsigned int taps, unsigned int delay, float gain, float leak)
{
	m_rx->setANRValues(taps, delay, gain, leak);
}

void CDttSP::setRXSquelchFlag(bool flag)
{
	m_rx->setSquelchFlag(flag);
}

void CDttSP::setRXSquelchThreshold(float threshold)
{
	m_rx->setSquelchThreshold(threshold);
}

void CDttSP::setRFGain(float gain)
{
	m_rx->setRFGain(gain);
}

void CDttSP::setMicGain(float gain)
{
	m_tx->setMicGain(gain);
}

void CDttSP::setPower(float power)
{
	m_tx->setPower(power);
}

void CDttSP::setANFFlag(bool flag)
{
	m_rx->setANFFlag(flag);
}

void CDttSP::setBANFFlag(bool flag)
{
	m_rx->setBANFFlag(flag);
}

void CDttSP::setANFValues(unsigned int taps, unsigned int delay, float gain, float leak)
{
	m_rx->setANFValues(taps, delay, gain, leak);
}

void CDttSP::setNBFlag(bool flag)
{
	m_rx->setNBFlag(flag);
}

void CDttSP::setNBThreshold(float threshold)
{
	m_rx->setNBThreshold(threshold);
}

void CDttSP::setNBSDROMFlag(bool flag)
{
	m_rx->setNBSDROMFlag(flag);
}

void CDttSP::setNBSDROMThreshold(float threshold)
{
	m_rx->setNBSDROMThreshold(threshold);
}

void CDttSP::setBinauralFlag(bool flag)
{
	m_rx->setBinauralFlag(flag);
}

void CDttSP::setAGCMode(AGCMODE mode)
{
	m_rx->setAGCMode(mode);
}

void CDttSP::setALCAttack(float attack)
{
	m_tx->setALCAttack(attack);
}

void CDttSP::setCarrierLevel(float level)
{
	m_tx->setAMCarrierLevel(level);
}

void CDttSP::setALCDecay(float decay)
{
	m_tx->setALCDecay(decay);
}

void CDttSP::setALCGainBottom(float gain)
{
	m_tx->setALCGainBottom(gain);
}

void CDttSP::setALCHangTime(float hang)
{
	m_tx->setALCHangTime(hang);
}

void CDttSP::setRXCorrectIQ(float phase, float gain)
{
	m_rx->setIQ(0.01F * phase, 1.0F + 0.01F * gain);
}

void CDttSP::setTXCorrectIQ(float phase, float gain)
{
	m_tx->setIQ(0.01F * phase, 1.0F + 0.01F * gain);
}

void CDttSP::setSpectrumType(SPECTRUMtype type)
{
	switch (type) {
		case SPEC_AUDIO:
			m_rx->setSpectrumType(SPEC_RX_POST_DET);
			m_tx->setSpectrumType(SPEC_TX_POST_ALC);
			break;
		case SPEC_IQ:
			m_rx->setSpectrumType(SPEC_RX_PRE_FILT);
			m_tx->setSpectrumType(SPEC_TX_POST_FILT);
			break;
	}
}

void CDttSP::setSpectrumWindowType(Windowtype window)
{
	m_spectrum->setWindowType(window);
}

void CDttSP::setSpectrumPolyphaseFlag(bool flag)
{
	m_spectrum->setPolyphaseFlag(flag);
}

void CDttSP::setCompressionFlag(bool flag)
{
	m_tx->setCompressionFlag(flag);
}

void CDttSP::setCompressionLevel(float level)
{
	m_tx->setCompressionLevel(level);
}

void CDttSP::setTRX(TRXMODE trx)
{
	m_update->Wait();

	switch (trx) {
		case TX:
			switch (m_tx->getMode()) {
				case CWU:
				case CWL:
					m_want = 0U;
					break;
				default:
					m_want = int(2.0F * m_sampleRate / 48000.0F);
					break;
			}
			break;

		case RX:
			m_want = int(1.0F * m_sampleRate / 48000.0F);
			break;
	}

	m_trxNext = trx;
	m_have    = 0U;

	if (m_state != RUN_SWITCH)
		m_stateLast = m_state;

	m_state = RUN_SWITCH;

	m_update->Post();
}

void CDttSP::setALCGainTop(float gain)
{
	m_tx->setALCGainTop(gain);
}

void CDttSP::getSpectrum(float *results)
{
	wxASSERT(results != NULL);

	m_spectrum->setScale(SPEC_PWR);

	m_update->Wait();
	m_spectrum->snapSpectrum();
	m_update->Post();

	m_spectrum->computeSpectrum(results);
}

void CDttSP::getPhase(float* results, unsigned int numpoints)
{
	wxASSERT(results != NULL);

	m_spectrum->setScale(SPEC_PWR);

	m_update->Wait();
	m_spectrum->snapScope();
	m_update->Post();

	m_spectrum->computeScopeComplex(results, numpoints);
}

void CDttSP::getScope(float* results, unsigned int numpoints)
{
	wxASSERT(results != NULL);

	m_spectrum->setScale(SPEC_PWR);

	m_update->Wait();
	m_spectrum->snapScope();
	m_update->Post();

	m_spectrum->computeScopeReal(results, numpoints);
}

float CDttSP::getMeter(METERTYPE mt)
{
	float returnval = -200.0F;

	switch (m_trx) {
		case RX:
			switch (mt) {
				case SIGNAL_STRENGTH:
					returnval = m_meter->getRXMeter(RX_SIGNAL_STRENGTH);
					break;
				case AVG_SIGNAL_STRENGTH:
					returnval = m_meter->getRXMeter(RX_AVG_SIGNAL_STRENGTH);
					break;
				case ADC_REAL:
					returnval = m_meter->getRXMeter(RX_ADC_REAL);
					break;
				case ADC_IMAG:
					returnval = m_meter->getRXMeter(RX_ADC_IMAG);
					break;
				default:
					returnval = -200.0F;
					break;
			}
			break;

		case TX:
			switch (mt) {
				case MIC:
					returnval = m_meter->getTXMeter(TX_MIC);
					break;
				case PWR:
					returnval = m_meter->getTXMeter(TX_PWR);
					break;
				case ALC:
					returnval = m_meter->getTXMeter(TX_ALC);
					break;
				case COMP:
					returnval = m_meter->getTXMeter(TX_COMP);
					break;
				default:
					returnval = -200.0F;
					break;
			}
			break;
	}

	return returnval;
}

void CDttSP::setDeviation(float value)
{
	m_tx->setFMDeviation(value);
	m_rx->setFMDeviation(value);
}

void CDttSP::ringBufferReset()
{
	m_update->Wait();

	m_inputI->clear();
	m_inputQ->clear();
	m_outputI->clear();
	m_outputQ->clear();

	m_update->Post();
}

// [pos]  0.0 <= pos <= 1.0
void CDttSP::setRXPan(float pos)
{
	if (pos < 0.0F || pos > 1.0F)
		return;

	m_rx->setAzim(pos);
}

float CDttSP::getTXOffset() const
{
	return m_tx->getOffset();
}

float CDttSP::getRXOffset() const
{
	return m_rx->getOffset();
}

void CDttSP::audioEntry(const float* input, float* output, unsigned int nframes)
{
	wxASSERT(input != NULL);
	wxASSERT(output != NULL);

	if (m_suspend) {
		::memset(output, 0x00, 2U * nframes * sizeof(float));
		return;
	}

	if (m_outputI->dataSpace() >= nframes && m_outputQ->dataSpace() >= nframes) {
		unsigned int i, j;
		// The following code is broken up in this manner to minimize cache hits
		for (i = 0U, j = 0U; i < nframes; i++, j += 2U)
			m_outputI->getData(&output[j], 1U);
		for (i = 0U, j = 1U; i < nframes; i++, j += 2U)
			m_outputQ->getData(&output[j], 1U);

		// Save the data in case we have an underrun later
		::memcpy(m_lastOutput, output, 2U * nframes * sizeof(float));
	} else {
		// m_inputI->clear();
		// m_inputQ->clear();
		m_outputI->clear();
		m_outputQ->clear();

		::memcpy(output, m_lastOutput, 2U * nframes * sizeof(float));
	}

	// input: copy from port to ring
	if (m_inputI->freeSpace() >= nframes && m_inputQ->freeSpace() >= nframes) {
		unsigned int i, j;
		// The following code is broken up in this manner to minimize cache hits
		for (i = 0U, j = 0U; i < nframes; i++, j += 2U)
			m_inputI->addData(&input[j], 1U);
		for (i = 0U, j = 1U; i < nframes; i++, j += 2U)
			m_inputQ->addData(&input[j], 1U);
	} else {
		m_inputI->clear();
		m_inputQ->clear();
		// m_outputI->clear();
		// m_outputQ->clear();

		wxLogError(wxT("Not enough space in the input ring buffer"));
	}

	// if enough accumulated in ring, fire dsp
	if (m_inputI->dataSpace() >= m_frames && m_inputQ->dataSpace() >= m_frames)
		m_buffer->Post();
}

void CDttSP::getHold()
{
	if (m_outputI->freeSpace() < m_frames) {
		// pathology
		wxLogError(wxT("Not enough space in the output ring buffer"));
	} else {
		m_outputI->addData(m_bufferI, m_frames);
		m_outputQ->addData(m_bufferQ, m_frames);
	}

	if (m_inputI->dataSpace() < m_frames) {
		// pathology
		::memset(m_bufferI, 0x00, m_frames * sizeof(float));
		::memset(m_bufferQ, 0x00, m_frames * sizeof(float));

		wxLogError(wxT("Not enough data in the input ring buffer"));
	} else {
		m_inputI->getData(m_bufferI, m_frames);
		m_inputQ->getData(m_bufferQ, m_frames);
	}
}

bool CDttSP::canHold()
{
	return m_inputI->dataSpace() >= m_frames && m_inputQ->dataSpace() >= m_frames;
}

void CDttSP::process()
{
	while (m_running) {
		m_buffer->Wait();

		while (canHold()) {
			getHold();

			m_update->Wait();

			switch (m_state) {
				case RUN_PLAY:
					runPlay();
					break;
				case RUN_SWITCH:
					runSwitch();
					break;
			}

			m_update->Post();
		}
	}
}

void CDttSP::runPlay()
{
	processSamples(m_bufferI, m_bufferQ, m_frames);
}

void CDttSP::runSwitch()
{
	if (m_have == 0) {
		// First time, apply ramp down
		for (unsigned int i = 0U; i < m_fade; i++) {
			float w = 1.0F - float(i) / float(m_fade);

			m_bufferI[i] *= w;
			m_bufferQ[i] *= w;
		}

		::memset(m_bufferI + m_fade, 0x00, m_tail);
		::memset(m_bufferQ + m_fade, 0x00, m_tail);

		m_have++;
	} else if (m_have < m_want) {
		// in medias res
		::memset(m_bufferI, 0x00, m_frames * sizeof(float));
		::memset(m_bufferQ, 0x00, m_frames * sizeof(float));

		m_have++;
	} else {
		// Last time, apply ramp up
		for (unsigned int i = 0U; i < m_fade; i++) {
			float w = float(i) / float(m_fade);

			m_bufferI[i] *= w;
			m_bufferQ[i] *= w;
		}

		m_trx   = m_trxNext;
		m_state = m_stateLast;

		m_want = 0U;
		m_have = 0U;
	}

	processSamples(m_bufferI, m_bufferQ, m_frames);
}

void CDttSP::processSamples(float* bufi, float* bufq, unsigned int n)
{
	wxASSERT(bufi != NULL);
	wxASSERT(bufq != NULL);

	switch (m_trx) {
		case RX:
			m_rx->process(bufi, bufq, n);
			break;
		case TX:
			m_tx->process(bufi, bufq, n);
			break;
	}
}

void CDttSP::setEqualiserFlag(bool flag)
{
	m_tx->setEqualiserFlag(flag);
}

void CDttSP::setEqualiserLevels(unsigned int n, const int* vals)
{
	m_tx->setEqualiserLevels(n, vals);
}
