/* FMDemod.cpp

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

#include "FMDemod.h"

CFMDemod::CFMDemod(float samprate, float f_initial, float f_lobound, float f_hibound, float f_bandwid, CXB* ivec, CXB* ovec) :
m_samprate(samprate),
m_ibuf(ivec),
m_obuf(ovec),
m_pllAlpha(0.0F),
m_pllBeta(0.0F),
m_pllFreqF(0.0F),
m_pllFreqL(0.0F),
m_pllFreqH(0.0F),
m_pllPhase(0.0F),
m_pllDelay(cxJ),
m_iirAlpha(0.0F),
m_lock(0.5F),
m_afc(0.0F),
m_cvt(0.0F)
{
	wxASSERT(ivec != NULL);
	wxASSERT(ovec != NULL);
	wxASSERT(samprate > 0.0F);

	float fac = 2.0 * M_PI / samprate;

	m_pllFreqF = f_initial * fac;
	m_pllFreqL = f_lobound * fac;
	m_pllFreqH = f_hibound * fac;

	m_iirAlpha = f_bandwid * fac;	/* arm filter */
	m_pllAlpha = m_iirAlpha * 0.3F;	/* pll bandwidth */
	m_pllBeta  = m_pllAlpha * m_pllAlpha * 0.25F;	/* second order term */

	m_cvt = 0.45F * samprate / (M_PI * f_bandwid);
}

CFMDemod::~CFMDemod()
{
}

void CFMDemod::setBandwidth(float f_lobound, float f_hibound)
{
	float fac = 2.0 * M_PI / m_samprate;

	m_pllFreqF = 0.0F;
	m_pllFreqL = f_lobound * fac;
	m_pllFreqH = f_hibound * fac;
}

void CFMDemod::setDeviation(float f_bandwid)
{
	float fac = 2.0 * M_PI / m_samprate;

	m_pllFreqF = 0.0F;

	m_iirAlpha = f_bandwid * fac;	/* arm filter */
	m_pllAlpha = m_iirAlpha * 0.3F;	/* pll bandwidth */
	m_pllBeta  = m_pllAlpha * m_pllAlpha * 0.25F;	/* second order term */

	m_cvt = 0.45F * m_samprate / (M_PI * f_bandwid);
}

void CFMDemod::demodulate()
{
	unsigned int n = CXBhave(m_ibuf);

	for (unsigned int i = 0; i < n; i++) {
		pll(CXBdata(m_ibuf, i));

		m_afc = float(0.9999 * m_afc + 0.0001F * m_pllFreqF);

		CXBreal(m_obuf, i) = CXBimag(m_obuf, i) = (m_pllFreqF - m_afc) * m_cvt;
	}

	CXBhave(m_obuf) = n;
}

void CFMDemod::pll(COMPLEX sig)
{
	COMPLEX z = Cmplx((float)cos(m_pllPhase), (float)sin (m_pllPhase));

	m_pllDelay.re = z.re * sig.re + z.im * sig.im;
	m_pllDelay.im = -z.im * sig.re + z.re * sig.im;

	float diff = (float)::atan2(m_pllDelay.im, m_pllDelay.re);

	m_pllFreqF += m_pllBeta * diff;

	if (m_pllFreqF < m_pllFreqL)
		m_pllFreqF = m_pllFreqL;
	if (m_pllFreqF > m_pllFreqH)
		m_pllFreqF = m_pllFreqH;

	m_pllPhase += m_pllFreqF + m_pllAlpha * diff;

	while (m_pllPhase >= 2.0 * M_PI)
		m_pllPhase -= float(2.0 * M_PI);

	while (m_pllPhase < 0.0F)
		m_pllPhase += float(2.0 * M_PI);
}

bool CFMDemod::hasBinaural() const
{
	return false;
}

bool CFMDemod::hasBlockANR() const
{
	return false;
}

bool CFMDemod::hasBlockANF() const
{
	return false;
}

bool CFMDemod::hasANR() const
{
	return false;
}

bool CFMDemod::hasANF() const
{
	return false;
}
