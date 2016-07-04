/* CXB.cpp

   creation, deletion, management for vectors and buffers

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

#include <cstdio>
#include <algorithm>
#include <cstring>
using std::min;
using std::max;

#include "CXB.h"
#include "Utils.h"

void CXBscl(CXB* buff, float scl)
{
	for (unsigned int i = 0; i < CXBhave(buff); i++)
		CXBdata(buff, i) = Cscl(CXBdata(buff, i), scl);
}

float CXBpeak(CXB* buff)
{
	float maxsam = 0.0F;
	for (unsigned int i = 0; i < CXBhave(buff); i++)
		maxsam = max(Cmag(CXBdata(buff, i)), maxsam);

	return maxsam;
}

/*------------------------------------------------------------------------*/
/* buffers (mainly i/o) */
/*------------------------------------------------------------------------*/
/* complex */

CXB* newCXB(unsigned int size, COMPLEX* base)
{
	CXB* p = new CXB;

	if (base != NULL) {
		CXBbase(p) = base;
		CXBmine(p) = false;
	} else {
		CXBbase(p) = new COMPLEX[size];
		::memset(CXBbase(p), 0x00, size * sizeof(COMPLEX));
		CXBmine(p) = true;
	}

	CXBsize(p) = size;
	CXBhave(p) = 0;

	return p;
}

void delCXB(CXB* p)
{
	if (p != NULL) {
		if (CXBmine(p))
			delete[] CXBbase(p);

		delete p;
	}
}

//========================================================================
// return normalization constant

float normalize_vec_COMPLEX(COMPLEX* z, unsigned int n)
{
	if (z != NULL && n > 0) {
		float big = -MONDO;

		for (unsigned int i = 0; i < n; i++) {
			float a = Cabs(z[i]);
			big = max(big, a);
		}

		if (big > 0.0F) {
			float scl = 1.0F / big;

			for (unsigned int i = 0; i < n; i++)
				z[i] = Cscl(z[i], scl);

			return scl;
		} else {
			return 0.0F;
		}
	} else {
		return 0.0F;
	}
}
