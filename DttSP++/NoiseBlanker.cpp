/* NoiseBlanker.cpp

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

#include <wx/wx.h>

#include "NoiseBlanker.h"

CNoiseBlanker::CNoiseBlanker(CXB* sigbuf, float threshold) :
m_sigbuf(sigbuf),
m_threshold(threshold),
m_aveSig(),
m_aveMag(1.0F),
m_delay(),
m_delayIndex(2),
m_sigIndex(0),
m_hangTime(0)
{
	wxASSERT(sigbuf != NULL);

	::memset(m_delay, 0x00, 8 * sizeof(COMPLEX));
}

CNoiseBlanker::~CNoiseBlanker()
{
}

void CNoiseBlanker::setThreshold(float threshold)
{
	m_threshold = threshold;
}

void CNoiseBlanker::blank()
{
	unsigned int n = CXBhave(m_sigbuf);

	for (unsigned int i = 0; i < n; i++) {
		float cmag = Cmag(CXBdata(m_sigbuf, i));

		m_delay[m_sigIndex] = CXBdata(m_sigbuf, i);
		m_aveMag = 0.999F * m_aveMag + 0.001F * cmag;

		if ((m_hangTime == 0) && (cmag > (m_threshold * m_aveMag)))
			m_hangTime = 7;

		if (m_hangTime > 0) {
			CXBdata(m_sigbuf, i) = cxzero;
			m_hangTime--;
		} else {
			CXBdata(m_sigbuf, i) = m_delay[m_delayIndex];
		}

		m_sigIndex   = (m_sigIndex + 7) & 7;
		m_delayIndex = (m_delayIndex + 7) & 7;
	}
}

void CNoiseBlanker::sdromBlank()
{
	unsigned int n = CXBhave(m_sigbuf);

	for (unsigned int i = 0; i < n; i++) {
		float cmag = Cmag(CXBdata(m_sigbuf, i));

		m_aveSig = Cadd(Cscl(m_aveSig, 0.75),

		Cscl(CXBdata(m_sigbuf, i), 0.25F));

		m_aveMag = 0.999F * m_aveMag + 0.001F * cmag;

		if (cmag > (m_threshold * m_aveMag))
			CXBdata(m_sigbuf, i) = m_aveSig;
	}
}
