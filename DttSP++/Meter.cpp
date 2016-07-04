/* Meter.cpp

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

#include <algorithm>
using std::min;
using std::max;

#include "Meter.h"
#include "Utils.h"

CMeter::CMeter() :
m_rxval(NULL),
m_rxmode(RX_SIGNAL_STRENGTH),
m_txval(NULL),
m_txmode(TX_PWR)
{
	m_rxval = new float[RXMETERTYPE_COUNT];
	m_txval = new float[TXMETERTYPE_COUNT];

	reset();
}

CMeter::~CMeter()
{
	delete[] m_rxval;
	delete[] m_txval;
}

void CMeter::reset()
{
	for (int i = 0; i < RXMETERTYPE_COUNT; i++)
		m_rxval[i] = -200.0F;

	for (int i = 0; i < TXMETERTYPE_COUNT; i++)
		m_txval[i] = -200.0F;
}

void CMeter::setRXMeter(RXMETERTAP tap, CXB* buf)
{
	wxASSERT(buf != NULL);

	COMPLEX* vec = CXBbase(buf);

	unsigned int len = CXBhave(buf);
	unsigned int i;

	float temp1, temp2;

	switch (tap) {
		case RXMETER_PRE_CONV:
			temp1 = m_rxval[RX_ADC_REAL];
			for (i = 0; i < len; i++)
				temp1 = (float)::max(float(::fabs(vec[i].re)), temp1);
			m_rxval[RX_ADC_REAL] = float(20.0 * ::log10(temp1 + 1e-10));

			temp1 = m_rxval[RX_ADC_IMAG];
			for (i = 0; i < len; i++)
				temp1 = (float)::max(float(::fabs(vec[i].im)), temp1);
			m_rxval[RX_ADC_IMAG] = float(20.0 * ::log10(temp1 + 1e-10));
			break;

		case RXMETER_POST_FILT:
			temp1 = 0.0F;
			for (i = 0; i < len; i++)
				temp1 += Csqrmag(vec[i]);
			// temp1 /= float(len);

			m_rxval[RX_SIGNAL_STRENGTH] = float(10.0 * ::log10(temp1 + 1e-20));

			temp1 = m_rxval[RX_SIGNAL_STRENGTH];
			temp2 = m_rxval[RX_AVG_SIGNAL_STRENGTH];
			m_rxval[RX_AVG_SIGNAL_STRENGTH] = float(0.95 * temp2 + 0.05 * temp1);
			break;

		default:
			break;
	}
}

void CMeter::setTXMeter(TXMETERTYPE type, CXB* buf)
{
	wxASSERT(buf != NULL);

	COMPLEX* vec = CXBbase(buf);

	unsigned int len = CXBhave(buf);

	float temp = 0.0F;

	for (unsigned int i = 0U; i < len; i++)
		temp += Cmag(vec[i]);

	m_txval[type] = temp / float(len);
}

float CMeter::getRXMeter(RXMETERTYPE type) const
{
 	return m_rxval[type];
}

float CMeter::getTXMeter(TXMETERTYPE type) const
{
	return m_txval[type];
}
