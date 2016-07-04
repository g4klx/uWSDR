/* AMDemod.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2006-2008 by Jonathan Naylor, G4KLX

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

#ifndef _amdemod_h
#define _amdemod_h

#include "Demod.h"
#include "Complex.h"
#include "CXB.h"


enum AMMode {
	AMdet,
	SAMdet
};

class CAMDemod : public IDemod {
    public:
	CAMDemod(float samprate, float f_initial, float f_lobound, float f_hibound, float f_bandwid, CXB* ivec, CXB* ovec, AMMode mode);
	virtual ~CAMDemod();

	virtual AMMode getMode() const;
	virtual void   setMode(AMMode mode);

	virtual void demodulate();

	virtual bool hasBinaural() const;

	virtual bool hasBlockANR() const;
	virtual bool hasBlockANF() const;

	virtual bool hasANR() const;
	virtual bool hasANF() const;

    private:
	CXB*    m_ibuf;
	CXB*    m_obuf;
	float   m_pllAlpha;
	float   m_pllBeta;
	float   m_pllFastAlpha;
	float   m_pllFreqF;
	float   m_pllFreqL;
	float   m_pllFreqH;
	float   m_pllPhase;
	float   m_pllIIRAlpha;
	COMPLEX m_pllDelay;
	float   m_lockCurr;
	float   m_lockPrev;
	float   m_dc;
	float   m_smooth;
	AMMode  m_mode;

	void pll(COMPLEX sig);
	float dem();
};

#endif
