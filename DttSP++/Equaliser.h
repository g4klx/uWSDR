/* Equaliser.h

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2013 by Jonathan Naylor, G4KLX

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

#ifndef _equaliser_h
#define _equaliser_h

#include "FilterOVSV.h"
#include "CXB.h"

class CEqualiser {
public:
	CEqualiser(float samprate, unsigned int pbits, CXB* vec);
	~CEqualiser();

	void setValues(unsigned int n, const int* vals);

	void process();

private:
	float        m_samprate;
	unsigned int m_pbits;
	CFilterOVSV* m_filter;
	CXB*         m_buf;
	COMPLEX*     m_in;
	COMPLEX*     m_out;
	COMPLEX*     m_zfvec;
};

#endif
