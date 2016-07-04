/* RX.cpp

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
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

#include <wx/wx.h>

#include <algorithm>
using std::min;
using std::max;

#include "RX.h"

CRX::CRX(unsigned int bufLen, unsigned int bits, float sampleRate, CMeter* meter, CSpectrum* spectrum, bool swapIQ) :
m_sampleRate(sampleRate),
m_meter(meter),
m_spectrum(spectrum),
m_type(SPEC_RX_POST_FILT),
m_swapIQ(swapIQ),
m_iBuf(NULL),
m_oBuf(NULL),
m_iq(NULL),
m_oscillator1(NULL),
m_oscillator2(NULL),
m_filter(NULL),
m_nb(NULL),
m_nbFlag(false),
m_nbSDROM(NULL),
m_nbSDROMFlag(false),
m_anr(NULL),
m_anrFlag(false),
m_anf(NULL),
m_anfFlag(false),
m_banr(NULL),
m_banrFlag(false),
m_banf(NULL),
m_banfFlag(false),
m_agc(NULL),
m_demodulator(NULL),
m_amDemodulator(NULL),
m_fmDemodulator(NULL),
m_ssbDemodulator(NULL),
m_squelch(NULL),
m_rfGain(0.0F),
m_mode(USB),
m_binFlag(false),
m_weaver(true),
m_freq(0.0),
m_lowFreq(0.0),
m_highFreq(0.0),
m_azim(),
m_tick(0UL)
{
	wxASSERT(meter != NULL);
	wxASSERT(spectrum != NULL);

	m_filter = new CFilterOVSV(bufLen, bits, sampleRate, -4800.0F, 4800.0F);

	m_iBuf = newCXB(m_filter->fetchSize(), m_filter->fetchPoint());
	m_oBuf = newCXB(m_filter->storeSize(), m_filter->storePoint());

	m_iq = new CCorrectIQ(m_iBuf);

	m_oscillator1 = new COscillator(m_iBuf, sampleRate);
	m_oscillator2 = new COscillator(m_oBuf, sampleRate);

	m_agc = new CAGC(agcLONG,	// mode kept around for control reasons alone
				    m_oBuf,	// input buffer
				    1.0F,	// Target output
				    2.0F,	// Attack time constant in ms
				    500.0F,	// Decay time constant in ms
				    1.0F,	// Slope
				    500.0F,	//Hangtime in ms
				    sampleRate,	// Sample rate
				    31622.8F,	// Maximum gain as a multipler, linear not dB
				    0.00001F,	// Minimum gain as a multipler, linear not dB
				    1.0F); 	// Set the current gain

	m_amDemodulator = new CAMDemod(sampleRate,	// float samprate
			 0.0F,	// float f_initial
			 -500.0F,	// float f_lobound,
			 500.0F,	// float f_hibound,
			 400.0F,	// float f_bandwid,
			 m_oBuf,	// COMPLEX *ivec,
			 m_oBuf,	// COMPLEX *ovec,
			 AMdet		// AM Mode AMdet == rectifier,
			 );			// SAMdet == synchronous detector

	m_fmDemodulator = new CFMDemod(sampleRate,	// float samprate
			 0.0F,	// float f_initial
			 -6000.0F,	// float f_lobound
			 6000.0F,	// float f_hibound
			 5000.0F,	// float f_bandwid
			 m_oBuf,	// COMPLEX *ivec
			 m_oBuf);	// COMPLEX *ovec

	m_ssbDemodulator = new CSSBDemod(m_oBuf, m_oBuf);

	m_demodulator = m_ssbDemodulator;

	m_anf = new CLMS(m_oBuf,	// CXB signal,
			    64,	// int delay,
			    0.01F,	// float adaptation_rate,
			    0.00001F,	// float leakage,
			    45,	// int adaptive_filter_size,
			    LMS_INTERFERENCE);

	m_banf = new CBlockLMS(m_oBuf, 0.00001f, 0.005f, BLMS_INTERFERENCE, bits);

	m_anr = new CLMS(m_oBuf,	// CXB signal,
			    64,	// int delay,
			    0.01f,	// float adaptation_rate,
			    0.00001f,	// float leakage,
			    45,	// int adaptive_filter_size,
			    LMS_NOISE);

	m_banr = new CBlockLMS(m_oBuf, 0.00001f, 0.005f, BLMS_NOISE, bits);

	m_nb = new CNoiseBlanker(m_iBuf, 3.3F);

	m_nbSDROM = new CNoiseBlanker(m_iBuf, 2.5F);

	m_squelch = new CSquelch(m_oBuf, -150.0F, 0.0F, bufLen - 48);

	float pos = 0.5;		// 0 <= pos <= 1, left->right
	float theta = float((1.0 - pos) * M_PI / 2.0);
	m_azim = Cmplx((float)::cos(theta), (float)::sin(theta));
}

CRX::~CRX()
{
	delete m_agc;
	delete m_nbSDROM;
	delete m_nb;
	delete m_anf;
	delete m_anr;
	delete m_banf;
	delete m_banr;
	delete m_amDemodulator;
	delete m_fmDemodulator;
	delete m_ssbDemodulator;
	delete m_oscillator1;
	delete m_oscillator2;
	delete m_iq;
	delete m_squelch;
	delCXB(m_oBuf);
	delCXB(m_iBuf);
	delete m_filter;
}

void CRX::process(float* bufi, float* bufq, unsigned int n)
{
	wxASSERT(bufi != NULL);
	wxASSERT(bufq != NULL);

	if (m_swapIQ) {
		for (unsigned int i = 0U; i < n; i++) {
			CXBreal(m_iBuf, i) = bufq[i];
			CXBimag(m_iBuf, i) = bufi[i];
		}
	} else {
		for (unsigned int i = 0U; i < n; i++) {
			CXBreal(m_iBuf, i) = bufi[i];
			CXBimag(m_iBuf, i) = bufq[i];
		}
	}
	CXBhave(m_iBuf) = n;

	CXBscl(m_iBuf, m_rfGain);

	if (m_nbFlag)
		m_nb->blank();

	if (m_nbSDROMFlag)
		m_nbSDROM->sdromBlank();

	meter(m_iBuf, RXMETER_PRE_CONV);

	m_iq->process();

	m_oscillator1->mix();

	spectrum(m_iBuf, SPEC_RX_PRE_FILT);

	if (m_tick == 0UL)
		m_filter->reset();

	m_filter->filter();
	CXBhave(m_oBuf) = CXBhave(m_iBuf);

	meter(m_oBuf, RXMETER_POST_FILT);
	spectrum(m_oBuf, SPEC_RX_POST_FILT);

	// Only active for the third method/zero-IF
	m_oscillator2->mix();

	if (m_squelch->isSquelch())
		m_squelch->doSquelch();
	else
		m_agc->process();

	spectrum(m_oBuf, SPEC_RX_POST_AGC);

	m_demodulator->demodulate();

	// The chosen demodulator selects which noise reduction options are valid for that mode
	if (m_binFlag && m_demodulator->hasBinaural()) {
		if (m_banrFlag && m_anrFlag && m_demodulator->hasBlockANR() && m_demodulator->hasANR())
			m_banr->process();

		if (m_banfFlag && m_anfFlag && m_demodulator->hasBlockANF() && m_demodulator->hasANF())
			m_banf->process();
	} else {
		if (m_anrFlag && m_demodulator->hasANR()) {
			if (m_banrFlag && m_demodulator->hasBlockANR())
				m_banr->process();
			else
				m_anr->process();
		}

		if (m_anfFlag && m_demodulator->hasANF()) {
			if (m_banfFlag && m_demodulator->hasBlockANF())
				m_banf->process();
			else
				m_anf->process();
		}

//		for (unsigned int i = 0; i < CXBhave(m_oBuf); i++)
//			CXBimag(m_oBuf, i) = CXBreal(m_oBuf, i);
	}

	n = CXBhave(m_oBuf);
	if (!m_squelch->isSet())
		m_squelch->noSquelch();

	spectrum(m_oBuf, SPEC_RX_POST_DET);

	for (unsigned int i = 0U; i < n; i++) {
		bufi[i] = CXBreal(m_oBuf, i);
		bufq[i] = CXBimag(m_oBuf, i);
	}

	m_tick++;
}

void CRX::meter(CXB* buf, RXMETERTAP tap)
{
	m_meter->setRXMeter(tap, buf);
}

void CRX::spectrum(CXB* buf, RXSPECTRUMtype type)
{
	if (type == m_type)
		m_spectrum->setData(buf);
}

void CRX::setMode(SDRMODE mode)
{
	m_mode = mode;

	if (m_weaver) {
		switch (mode) {
			case AM:
				m_amDemodulator->setMode(AMdet);
				m_oscillator2->setFrequency(-INV_FREQ);
				m_demodulator = m_amDemodulator;
				break;

			case SAM:
				m_amDemodulator->setMode(SAMdet);
				m_oscillator2->setFrequency(-INV_FREQ);
				m_demodulator = m_amDemodulator;
				break;

			case FMN:
				m_oscillator2->setFrequency(-INV_FREQ);
				m_demodulator = m_fmDemodulator;
				break;

			case USB:
			case CWU:
			case DIGU:
				m_oscillator2->setFrequency(-INV_FREQ);
				m_demodulator = m_ssbDemodulator;
				break;

			case LSB:
			case CWL:
			case DIGL:
				m_oscillator2->setFrequency(INV_FREQ);
				m_demodulator = m_ssbDemodulator;
				break;
		}
	} else {
		switch (mode) {
			case AM:
				m_amDemodulator->setMode(AMdet);
				m_oscillator2->setFrequency(0.0);
				m_demodulator = m_amDemodulator;
				break;

			case SAM:
				m_amDemodulator->setMode(SAMdet);
				m_oscillator2->setFrequency(0.0);
				m_demodulator = m_amDemodulator;
				break;

			case FMN:
				m_oscillator2->setFrequency(0.0);
				m_demodulator = m_fmDemodulator;
				break;

			case USB:
			case LSB:
			case CWL:
			case CWU:
			case DIGU:
			case DIGL:
				m_oscillator2->setFrequency(0.0);
				m_demodulator = m_ssbDemodulator;
				break;
		}
	}
}

void CRX::setWeaver(bool flag)
{
	m_weaver = flag;

	setFrequency(m_freq);
	setFilter(m_lowFreq, m_highFreq);
	setMode(m_mode);
}

void CRX::setFilter(double lowFreq, double highFreq)
{
	m_lowFreq  = lowFreq;
	m_highFreq = highFreq;

	if (m_weaver) {
		if (m_mode == LSB || m_mode == CWL || m_mode == DIGL)
			m_filter->setFilter(lowFreq + INV_FREQ, highFreq + INV_FREQ);
		else
			m_filter->setFilter(lowFreq - INV_FREQ, highFreq - INV_FREQ);
	} else {
		m_filter->setFilter(lowFreq, highFreq);
	}

	m_fmDemodulator->setBandwidth(lowFreq, highFreq);
}

void CRX::setFrequency(double freq)
{
	m_freq = freq;

	m_oscillator1->setFrequency(freq);
}

void CRX::setSquelchFlag(bool flag)
{
	m_squelch->setFlag(flag);
}

void CRX::setSquelchThreshold(float threshold)
{
	m_squelch->setThreshold(threshold);
}

void CRX::setRFGain(float gain)
{
	m_rfGain = gain;
}

void CRX::setFMDeviation(float deviation)
{
	m_fmDemodulator->setDeviation(deviation);
}

void CRX::setIQ(float phase, float gain)
{
	m_iq->setPhase(phase);
	m_iq->setGain(gain);
}

void CRX::setANFFlag(bool flag)
{
	m_anfFlag = flag;
}

void CRX::setBANFFlag(bool flag)
{
	m_banfFlag = flag;
}

void CRX::setANRValues(unsigned int adaptiveFilterSize, unsigned int delay, float adaptationRate, float leakage)
{
	m_anr->setAdaptiveFilterSize(adaptiveFilterSize);
	m_anr->setDelay(delay);
	m_anr->setAdaptationRate(adaptationRate);
	m_anr->setLeakage(leakage);

	m_banr->setAdaptationRate(::min(0.1F * adaptationRate, 0.0002F));
}

void CRX::setANRFlag(bool flag)
{
	m_anrFlag = flag;
}

void CRX::setBANRFlag(bool flag)
{
	m_banrFlag = flag;
}

void CRX::setANFValues(unsigned int adaptiveFilterSize, unsigned int delay, float adaptationRate, float leakage)
{
	m_anf->setAdaptiveFilterSize(adaptiveFilterSize);
	m_anf->setDelay(delay);
	m_anf->setAdaptationRate(adaptationRate);
	m_anf->setLeakage(leakage);

	m_banf->setAdaptationRate(::min(0.1F * adaptationRate, 0.0002F));
}

void CRX::setNBFlag(bool flag)
{
	m_nbFlag = flag;
}

void CRX::setNBThreshold(float threshold)
{
	m_nb->setThreshold(threshold);
}

void CRX::setNBSDROMFlag(bool flag)
{
	m_nbSDROMFlag = flag;
}

void CRX::setNBSDROMThreshold(float threshold)
{
	m_nbSDROM->setThreshold(threshold);
}

void CRX::setBinauralFlag(bool flag)
{
	m_binFlag = flag;
}

void CRX::setAGCMode(AGCMODE mode)
{
	m_agc->setMode(mode);
}

void CRX::setAzim(float azim)
{
	float theta = float((1.0 - azim) * M_PI / 2.0);

	m_azim = Cmplx((float)::cos(theta), (float)::sin(theta));
}

void CRX::setSpectrumType(RXSPECTRUMtype type)
{
	m_type = type;
}

float CRX::getOffset() const
{
	if (m_weaver) {
		if (m_mode == LSB || m_mode == CWL || m_mode == DIGL)
			return -INV_FREQ;
		else
			return INV_FREQ;
	} else {
		return 0.0F;
	}
}
