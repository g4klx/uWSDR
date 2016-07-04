/* SSBDemod.cpp

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

#include "SSBDemod.h"

CSSBDemod::CSSBDemod(CXB* ibuf, CXB* obuf) :
m_ibuf(ibuf),
m_obuf(obuf)
{
	wxASSERT(ibuf != NULL);
	wxASSERT(obuf != NULL);
}

CSSBDemod::~CSSBDemod()
{
}

void CSSBDemod::demodulate()
{
	if (m_ibuf == m_obuf)
		return;

	unsigned int n = CXBhave(m_ibuf);

	for (unsigned int i = 0; i < n; i++)
		CXBdata(m_obuf, i) = CXBdata(m_ibuf, i);

	CXBhave(m_obuf) = n;
}

bool CSSBDemod::hasBinaural() const
{
	return true;
}

bool CSSBDemod::hasBlockANR() const
{
	return true;
}

bool CSSBDemod::hasBlockANF() const
{
	return true;
}

bool CSSBDemod::hasANR() const
{
	return true;
}

bool CSSBDemod::hasANF() const
{
	return true;
}

