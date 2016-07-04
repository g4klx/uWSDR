/* LMS.h 

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

#ifndef _lms_h
#define _lms_h

#include "CXB.h"
#include "CXOps.h"
#include "fftw3.h"


enum {
	LMS_INTERFERENCE,
	LMS_NOISE
};


class CLMS {
    public:
	CLMS(CXB* signal, unsigned int delay, float adaptationRate, float leakage, unsigned int adaptiveFilterSize, unsigned int filterType);
	~CLMS();

	void setAdaptationRate(float adaptationRate);
	void setAdaptiveFilterSize(unsigned int adaptiveFilterSize);
	void setDelay(unsigned int delay);
	void setLeakage(float leakage);

	void process();

	void processInterference();
	void processNoise();

    private:
	CXB*         m_signal;				/* Signal Buffer */
	unsigned int m_delay;				/* Total delay between current sample and filter */
	float        m_adaptationRate;		/* Adaptation rate for the LMS stochastic gradient */
	float        m_leakage;				/* Exponential decay constant for filter coefficients */
	unsigned int m_adaptiveFilterSize;	/* number taps in adaptive filter */
	unsigned int m_filterType;			/* Filter type */
	float*       m_delayLine;			/* Delay Line circular buffer for holding samples */
	float*       m_adaptiveFilter;		/* Filter coefficients */
	unsigned int m_delayLinePtr;		/* Pointer for next sample into the delay line */
	unsigned int m_size;				/* Delay line size */
	unsigned int m_mask;				/* Mask for circular buffer */
};

#endif
