/* BlockLMS.h 

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

#ifndef _blocklms_h
#define _blocklms_h

#include "CXB.h"
#include "CXOps.h"
#include "fftw3.h"


enum {
	BLMS_INTERFERENCE,
	BLMS_NOISE
};


class CBlockLMS {
    public:
	CBlockLMS(CXB* signal, float adaptationRate, float leakRate, unsigned int filterType, unsigned int pbits);
	virtual ~CBlockLMS();

	void setAdaptationRate(float adaptationRate);

	void process();

    private:
	CXB*         m_signal;
	float        m_adaptationRate;
	float        m_leakRate;
	unsigned int m_filterType;
	COMPLEX*     m_delayLine;
	COMPLEX*     m_y;
	COMPLEX*     m_wHat;
	COMPLEX*     m_xHat;
	COMPLEX*     m_yHat;
	COMPLEX*     m_error;
	COMPLEX*     m_errHat;
	COMPLEX*     m_update1;
	COMPLEX*     m_update2;
	fftwf_plan   m_wPlan;
	fftwf_plan   m_xPlan;
	fftwf_plan   m_yPlan;
	fftwf_plan   m_errHatPlan;
	fftwf_plan   m_updPlan;
};

#endif
