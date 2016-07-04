/* Squelch.cpp

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004,2005,2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
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

#include "Squelch.h"

CSquelch::CSquelch(CXB* buf, float threshold, float offset, unsigned int num) :
m_buf(buf),
m_thresh(threshold),
m_offset(offset),
m_num(num),
m_power(0.0F),
m_set(false),
m_running(false),
m_flag(false)
{
	wxASSERT(m_buf != NULL);
}

CSquelch::~CSquelch()
{
}

bool CSquelch::isSquelch()
{
	if (m_flag) {
		unsigned int n = CXBhave(m_buf);

		m_power = 0.0F;
		for (unsigned int i = 0; i < n; i++)
			m_power += Csqrmag(CXBdata(m_buf, i));

		return (m_offset + 10.0F * (float)::log10(m_power + 1e-17)) < m_thresh;
	} else {
		m_set = false;

		return false;
	}
}

void CSquelch::doSquelch()
{
	m_set = true;

	if (!m_running) {
		unsigned int n = CXBhave(m_buf) - m_num;

		for (unsigned int i = 0; i < m_num; i++)
			CXBdata(m_buf, i) = Cscl(CXBdata(m_buf, i), 1.0F - float(i) / float(m_num));

		::memset((CXBbase(m_buf) + m_num), 0x00, n * sizeof(COMPLEX));
		m_running = true;
	} else {
		::memset(CXBbase(m_buf), 0x00, CXBhave(m_buf) * sizeof(COMPLEX));
	}
}

void CSquelch::noSquelch()
{
	if (m_running) {
		for (unsigned int i = 0; i < m_num; i++)
			CXBdata(m_buf, i) = Cscl(CXBdata(m_buf, i), float(i) / float(m_num));

		m_running = false;
	}
}

bool CSquelch::isSet() const
{
	return m_set;
}

void CSquelch::setFlag(bool flag)
{
	m_flag = flag;
}

void CSquelch::setThreshold(float threshold)
{
	m_thresh = threshold;
}
