/* AGC.h

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

#ifndef _agc_h
#define _agc_h

#include "Complex.h"
#include "CXB.h"


const int FASTLEAD = 72;

enum AGCMODE {
	agcOFF,
	agcLONG,
	agcSLOW,
	agcMED,
	agcFAST
};

class CAGC {
    public:
	CAGC(AGCMODE mode, CXB* buff, float limit, float attack,
	     float decay, float slope, float hangtime, float samprate, float maxGain,
		 float minGain, float curgain);
	~CAGC();

	void process();

	float getGain() const;

	void setMode(AGCMODE mode);
	void setGain(float gain);
	void setHangTime(float time);
	void setGainTop(float gain);
	void setGainBottom(float gain);
	void setAttack(float attack);
	void setDecay(float decay);

    private:
	AGCMODE      m_mode;
	float        m_samprate;
	float        m_gainTop;
	float        m_gainNow;
	float        m_gainFastNow;
	float        m_gainBottom;
	float        m_gainLimit;
	float        m_gainFix;
	float        m_attack;
	float        m_oneMAttack;
	float        m_decay;
	float        m_oneMDecay;
	float        m_slope;
	float        m_fastAttack;
	float        m_oneMFastAttack;
	float        m_fastDecay;
	float        m_oneMFastDecay;
	float        m_hangTime;
	float        m_hangThresh;
	float        m_fastHangTime;		//wa6ahl:  added to structure
	COMPLEX*     m_circ;
	CXB*         m_buff;
	unsigned int m_mask;
	unsigned int m_index;
	unsigned int m_sndex;
	unsigned int m_hangIndex;
	unsigned int m_fastIndex;
	unsigned int m_fastHang;			//wa6ahl:  added to structure
};

#endif
