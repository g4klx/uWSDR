/* Oscillator.cpp

This routine implements a common fixed-frequency oscillator

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

#include "Oscillator.h"

COscillator::COscillator(CXB* buf, float sampleRate, double frequency) :
m_buf(buf),
m_frequency(frequency),
m_sampleRate(sampleRate),
m_cosVal(1.0F),
m_sinVal(0.0F),
m_cosDelta(0.0F),
m_sinDelta(0.0F)
{
	wxASSERT(buf != NULL);
	wxASSERT(sampleRate > 0.0F);

	setFrequency(frequency);
}

COscillator::~COscillator()
{
}

void COscillator::setFrequency(double frequency)
{
	float delta = frequency / m_sampleRate * 2.0 * M_PI;

	m_frequency = frequency;
	m_cosDelta  = ::cos(delta);
	m_sinDelta  = ::sin(delta);
}

void COscillator::oscillate()
{
	unsigned int len = CXBhave(m_buf);

	if (m_frequency == 0.0 || len == 0)
		return;

	for (unsigned int i = 0; i < len; i++) {
		float tmpVal = m_cosVal * m_cosDelta - m_sinVal * m_sinDelta;
		m_sinVal = m_cosVal * m_sinDelta + m_sinVal * m_cosDelta;
		m_cosVal = tmpVal;

		CXBdata(m_buf, i) = Cmplx(m_cosVal, m_sinVal);
	}
}

void COscillator::mix()
{
	unsigned int len = CXBhave(m_buf);

	if (m_frequency == 0.0 || len == 0)
		return;

	for (unsigned int i = 0; i < len; i++) {
		float tmpVal = m_cosVal * m_cosDelta - m_sinVal * m_sinDelta;
		m_sinVal = m_cosVal * m_sinDelta + m_sinVal * m_cosDelta;
		m_cosVal = tmpVal;

		CXBdata(m_buf, i) = Cmul(CXBdata(m_buf, i), Cmplx(m_cosVal, m_sinVal));
	}
}

