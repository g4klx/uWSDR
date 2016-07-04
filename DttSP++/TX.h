/* TX.h

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

#ifndef _tx_h
#define _tx_h

#include "Defs.h"
#include "CXB.h"
#include "CorrectIQ.h"
#include "DCBlock.h"
#include "Oscillator.h"
#include "FilterOVSV.h"
#include "Mod.h"
#include "AMMod.h"
#include "FMMod.h"
#include "SSBMod.h"
#include "Squelch.h"
#include "AGC.h"
#include "SpeechProc.h"
#include "Meter.h"
#include "Spectrum.h"
#include "Equaliser.h"


class CTX {
public:
	CTX(unsigned int bufLen, unsigned int bits, float sampleRate, CMeter* meter, CSpectrum* spectrum, bool swapIQ);
	~CTX();

	void process(float* bufi, float* bufq, unsigned int n);

	SDRMODE getMode() const;

	void setMode(SDRMODE mode);

	void setWeaver(bool flag);

	void setDCBlockFlag(bool flag);

	void setFilter(double lowFreq, double highFreq);

	void setFrequency(double freq);

	void setAMCarrierLevel(float level);

	void setFMDeviation(float deviation);

	void setIQ(float phase, float gain);

	void setALCAttack(float attack);
	void setALCDecay(float decay);
	void setALCGainBottom(float bottom);
	void setALCGainTop(float top);
	void setALCHangTime(float hang);

	void setCompressionFlag(bool flag);
	void setCompressionLevel(float level);

	void setSpectrumType(TXSPECTRUMtype type);

	void setEqualiserFlag(bool flag);
	void setEqualiserLevels(unsigned int n, const int* vals);

	void setMicGain(float gain);
	void setPower(float power);

	float getOffset() const;

private:
	float          m_sampleRate;

	CMeter*        m_meter;
	CSpectrum*     m_spectrum;
	TXSPECTRUMtype m_type;

	bool           m_swapIQ;

	CXB*           m_iBuf;
	CXB*           m_oBuf;

	CCorrectIQ*    m_iq;

	CDCBlock*      m_dcBlock;
	bool           m_dcBlockFlag;

	COscillator*   m_oscillator1;
	COscillator*   m_oscillator2;

	CFilterOVSV*   m_filter;

	IMod*          m_modulator;
	CAMMod*        m_amModulator;
	CFMMod*        m_fmModulator;
	CSSBMod*       m_ssbModulator;

	CAGC*          m_alc;
	float          m_micGain;
	float          m_power;

	CSpeechProc*   m_speechProc;
	bool           m_speechProcFlag;

	CEqualiser*    m_equaliser;
	bool           m_equaliserFlag;

	SDRMODE        m_mode;
	bool           m_weaver;

	double         m_freq;
	double         m_lowFreq;
	double         m_highFreq;

	unsigned long  m_tick;

	void meter(CXB* buf, TXMETERTYPE type);
	void spectrum(CXB* buf, TXSPECTRUMtype type);
};

#endif
