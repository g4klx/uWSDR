/* Spectrum.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2006-2007 by Jonathan Naylor, G4KLX

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

#ifndef _spectrum_h
#define _spectrum_h

#include "Window.h"
#include "CXB.h"
#include "fftw3.h"


enum SPECTRUMscale {
	SPEC_MAG,
	SPEC_PWR
};

enum SPECTRUMtype {
	SPEC_AUDIO,
	SPEC_IQ,
};

enum RXSPECTRUMtype {
	SPEC_RX_PRE_FILT,
	SPEC_RX_POST_FILT,
	SPEC_RX_POST_AGC,
	SPEC_RX_POST_DET
};

enum TXSPECTRUMtype {
	SPEC_TX_MIC,
	SPEC_TX_POST_COMP,
	SPEC_TX_POST_ALC,
	SPEC_TX_POST_FILT
};


class CSpectrum {
    public:
	CSpectrum(unsigned int size, unsigned int planbits, SPECTRUMscale scale);
	~CSpectrum();

	void setScale(SPECTRUMscale scale);

	void setWindowType(Windowtype type);

	void setPolyphaseFlag(bool setit);

	void setData(CXB* buf);

	void reinitSpectrum();

	void snapSpectrum();
	void computeSpectrum(float* spectrum);

	void snapScope();
	void computeScopeReal(float* results, unsigned int numpoints);
	void computeScopeComplex(float* results, unsigned int numpoints);

    private:
	CXB*          m_accum;
	CXB*          m_timebuf;
	CXB*          m_freqbuf;
	unsigned int  m_fill;
	SPECTRUMscale m_scale;
	unsigned int  m_size;
	unsigned int  m_mask;
	Windowtype    m_winType;
	float*        m_window;
	unsigned int  m_planbits;
	fftwf_plan    m_plan;
	bool          m_polyphase;
};

#endif
