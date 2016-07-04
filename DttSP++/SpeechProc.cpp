/* SpeechProc.cpp

  This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY, Phil Harman, VK6APH
Copyright (C) 2006-2008,2013 by Jonathan Naylor, G4KLX

Based on Visual Basic code for SDR by Phil Harman

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

#include <algorithm>
using std::min;
using std::max;

#include "SpeechProc.h"
#include "Utils.h"

CSpeechProc::CSpeechProc(float k, float maxCompression, CXB* spdat) :
m_CG(NULL),
m_buf(spdat),
m_lastCG(1.0F),
m_k(k),
m_maxGain(0.0F),
m_fac(0.0F)
{
	wxASSERT(spdat != NULL);

	unsigned int size = CXBsize(spdat);

	m_CG  = new float[size + 1];
	::memset(m_CG, 0x00, (size + 1) * sizeof(float));

	setCompression(maxCompression);
}

CSpeechProc::~CSpeechProc()
{
	delete[] m_CG;
}

void CSpeechProc::setCompression(float compression)
{
	m_maxGain = ::pow(10.0, compression * 0.05);

	m_fac = float((((0.0000401002 * compression) - 0.0032093390) * compression + 0.0612862687) * compression + 0.9759745718);
}

void CSpeechProc::process()
{
	unsigned int n = CXBhave(m_buf);

	if (m_maxGain == 1.0F)
		return;

	// K was 0.4 in VB version, this value is better, perhaps due to filters that follow?
	float r = 0.0;
	for (unsigned int i = 0U; i < n; i++)
		r = ::max(r, Cmag(CXBdata(m_buf, i)));	// find the peak magnitude value in the sample buffer

	m_CG[0] = m_lastCG;	// restore from last time

	for (unsigned int i = 1U; i <= n; i++) {
		float mag = Cmag(CXBdata(m_buf, i - 1));

		if (mag != 0.0F) {
			float val = m_CG[i - 1] * (1.0F - m_k) + (m_k * r / mag);	// Frerking's formula

			m_CG[i] = (val > m_maxGain) ? m_maxGain : val;
		} else {
			m_CG[i] = m_maxGain;
		}
	}

	m_lastCG = m_CG[n];	// save for next time

	for (unsigned int i = 0U; i < n; i++)	// multiply each sample by its gain constant
		CXBdata(m_buf, i) = Cscl(CXBdata(m_buf, i), float(m_CG[i] / (m_fac * ::pow(m_maxGain, 0.25F))));
}
