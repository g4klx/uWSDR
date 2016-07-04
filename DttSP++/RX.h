/* RX.h

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

#ifndef _rx_h
#define _rx_h

#include "Defs.h"
#include "CXB.h"
#include "Meter.h"
#include "Spectrum.h"
#include "CorrectIQ.h"
#include "Oscillator.h"
#include "FilterOVSV.h"
#include "NoiseBlanker.h"
#include "LMS.h"
#include "BlockLMS.h"
#include "AGC.h"
#include "Demod.h"
#include "AMDemod.h"
#include "FMDemod.h"
#include "SSBDemod.h"
#include "Squelch.h"

class CRX {
public:
	CRX(unsigned int bufLen, unsigned int bits, float sampleRate, CMeter* meter, CSpectrum* spectrum, bool swapIQ);
	~CRX();

	void process(float* bufi, float* bufq, unsigned int n);

	void setMode(SDRMODE mode);

	void setWeaver(bool flag);

	void setFilter(double lowFreq, double highFreq);

	void setFrequency(double freq);

	void setSquelchFlag(bool flag);
	void setSquelchThreshold(float threshold);

	void setFMDeviation(float deviation);

	void setIQ(float phase, float gain);

	void setANFFlag(bool flag);
	void setBANRFlag(bool flag);
	void setANFValues(unsigned int adaptiveFilterSize, unsigned int delay, float adaptationRate, float leakage);

	void setANRFlag(bool flag);
	void setBANFFlag(bool flag);
	void setANRValues(unsigned int adaptiveFilterSize, unsigned int delay, float adaptationRate, float leakage);

	void setNBFlag(bool flag);
	void setNBThreshold(float threshold);

	void setNBSDROMFlag(bool flag);
	void setNBSDROMThreshold(float threshold);

	void setBinauralFlag(bool flag);

	void setAGCMode(AGCMODE mode);

	void setAzim(float azim);

	void setSpectrumType(RXSPECTRUMtype type);

	void setRFGain(float gain);

	float getOffset() const;

private:
	float          m_sampleRate;

	CMeter*        m_meter;
	CSpectrum*     m_spectrum;
	RXSPECTRUMtype m_type;

	bool           m_swapIQ;

	CXB*           m_iBuf;
	CXB*           m_oBuf;

	CCorrectIQ*    m_iq;

	COscillator*   m_oscillator1;
	COscillator*   m_oscillator2;

	CFilterOVSV*   m_filter;

	CNoiseBlanker* m_nb;
	bool           m_nbFlag;

	CNoiseBlanker* m_nbSDROM;
	bool           m_nbSDROMFlag;

	CLMS*          m_anr;
	bool           m_anrFlag;

	CLMS*          m_anf;
	bool           m_anfFlag;

	CBlockLMS*     m_banr;
	bool           m_banrFlag;

	CBlockLMS*     m_banf;
	bool           m_banfFlag;

	CAGC*          m_agc;

	IDemod*        m_demodulator;
	CAMDemod*      m_amDemodulator;
	CFMDemod*      m_fmDemodulator;
	CSSBDemod*     m_ssbDemodulator;

	CSquelch*      m_squelch;
	float          m_rfGain;

	SDRMODE        m_mode;

	bool           m_binFlag;

	bool           m_weaver;

	double         m_freq;
	double         m_lowFreq;
	double         m_highFreq;

	COMPLEX        m_azim;
	unsigned long  m_tick;

	void meter(CXB* buf, RXMETERTAP tap);
	void spectrum(CXB* buf, RXSPECTRUMtype type);
};

#endif
