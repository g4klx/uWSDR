/* FilterOVSV.h

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

#ifndef _filterovsv_h
#define _filterovsv_h

#include "Complex.h"
#include "fftw3.h"


class CFilterOVSV {
    public:
	CFilterOVSV(unsigned int bufLen, unsigned int pbits, float sampleRate, float lowFreq, float highFreq);
	~CFilterOVSV();

	COMPLEX* fetchPoint();
	COMPLEX* storePoint();

	unsigned int fetchSize();
	unsigned int storeSize();

	COMPLEX* getZFvec();

	void reset();

	void setFilter(float lowFreq, float highFreq);

	void filter();

    private:
	unsigned int m_pbits;
	float        m_samprate;
	unsigned int m_bufLen;
	COMPLEX*     m_zfvec;
	COMPLEX*     m_zivec;
	COMPLEX*     m_zovec;
	COMPLEX*     m_zrvec;
	fftwf_plan   m_pfwd;
	fftwf_plan   m_pinv;
	float        m_scale;
};

#endif
