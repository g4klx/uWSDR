/* AGC.cpp

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
using std::max;
using std::min;

#include "AGC.h"
#include "Utils.h"

CAGC::CAGC(AGCMODE mode, CXB* buff, float limit, float attack,
		   float decay, float slope, float hangtime, float samprate, float maxGain,
		   float minGain, float curGain) :
m_mode(mode),
m_samprate(samprate),
m_gainTop(maxGain),
m_gainNow(curGain),
m_gainFastNow(curGain),
m_gainBottom(minGain),
m_gainLimit(limit),
m_gainFix(10.0F),
m_attack(0.0F),
m_oneMAttack(0.0F),
m_decay(0.0F),
m_oneMDecay(0.0F),
m_slope(slope),
m_fastAttack(0.0F),
m_oneMFastAttack(0.0F),
m_fastDecay(0.0F),
m_oneMFastDecay(0.0F),
m_hangTime(hangtime * 0.001F),
m_hangThresh(minGain),
m_fastHangTime(48.0F * 0.001F),		//wa6ahl:  added to structure
m_circ(NULL),
m_buff(buff),
m_mask(0U),
m_index(0U),
m_sndex(0U),
m_hangIndex(0U),
m_fastIndex(FASTLEAD),
m_fastHang(0U)				//wa6ahl:  added to structure
{
	wxASSERT(buff != NULL);
	wxASSERT(samprate > 0.0F);

	m_mask = 2U * CXBsize(buff);

	setAttack(attack);

	setDecay(decay);

	m_fastAttack     = float(1.0 - ::exp(-1000.0 / (0.2 * samprate)));
	m_oneMFastAttack = (float)::exp(-1000.0 / (0.2 * samprate));

	m_fastDecay     = float(1.0 - ::exp(-1000.0 / (3.0 * samprate)));
	m_oneMFastDecay = (float)::exp(-1000.0 / (3.0 * samprate));

	m_circ = new COMPLEX[m_mask];
	::memset(m_circ, 0x00, m_mask * sizeof(COMPLEX));

	m_mask -= 1;
}

CAGC::~CAGC()
{
	delete[] m_circ;
}

// FIXME check braces
void CAGC::process()
{
	unsigned int n = CXBhave(m_buff);

	unsigned int hangTime     = (unsigned int)(m_samprate * m_hangTime);
	unsigned int fastHangTime = (unsigned int)(m_samprate * m_fastHangTime);

	float hangThresh = 0.0F;

	if (m_hangThresh > 0.0F)
		hangThresh = m_gainTop * m_hangThresh + m_gainBottom * (1.0F - m_hangThresh);

	if (m_mode == agcOFF) {
		for (unsigned int i = 0U; i < n; i++)
			CXBdata(m_buff, i) = Cscl(CXBdata(m_buff, i), m_gainFix);

		return;
	}

	for (unsigned int i = 0U; i < n; i++) {
		m_circ[m_index] = CXBdata(m_buff, i);	// Drop sample into circular buffer
		float tmp = 1.1F * Cmag(m_circ[m_index]);

		if (tmp != 0.0F)
			tmp = m_gainLimit / tmp;	// if not zero sample, calculate gain
		else
			tmp = m_gainNow;		// update. If zero, then use old gain

		if (tmp < hangThresh)
			m_hangIndex = hangTime;

		if (tmp >= m_gainNow) {
			if (m_hangIndex++ > hangTime)
				m_gainNow = m_oneMDecay * m_gainNow + m_decay * min(m_gainTop, tmp);
		} else {
			m_hangIndex = 0;
			m_gainNow = m_oneMAttack * m_gainNow + m_attack * max(tmp, m_gainBottom);
		}

		tmp = 1.2F * Cmag(m_circ[m_fastIndex]);

		if (tmp != 0.0)
			tmp = m_gainLimit / tmp;
		else
			tmp = m_gainFastNow;

		if (tmp > m_gainFastNow) {
			if (m_fastHang++ > fastHangTime)
				m_gainFastNow = min(m_oneMFastDecay * m_gainFastNow + m_fastDecay * min(m_gainTop, tmp), m_gainTop);
		} else {
			m_fastHang = 0U;
			m_gainFastNow = max(m_oneMFastAttack * m_gainFastNow + m_fastAttack * max(tmp, m_gainBottom), m_gainBottom);
		}

		m_gainFastNow = max(min(m_gainFastNow, m_gainTop), m_gainBottom);
		m_gainNow     = max(min(m_gainNow,     m_gainTop), m_gainBottom);

		CXBdata(m_buff, i) = Cscl(m_circ[m_sndex], min(m_gainFastNow, min(m_slope * m_gainNow, m_gainTop)));

		m_index = (m_index + m_mask) & m_mask;
		m_sndex = (m_sndex + m_mask) & m_mask;

		m_fastIndex = (m_fastIndex + m_mask) & m_mask;
	}
}

float CAGC::getGain() const
{
	return m_gainNow;
}

void CAGC::setMode(AGCMODE mode)
{
	m_mode = mode;
	m_attack = float(1.0 - ::exp(-500.0 / m_samprate));
	m_oneMAttack = 1.0F - m_attack;
	m_hangIndex = 0;
	m_index = 0;
	m_sndex = (unsigned int)(m_samprate * 0.006F);
	m_fastIndex = FASTLEAD;

	switch (mode) {
		case agcOFF:
			break;

		case agcSLOW:
			m_hangTime = 0.5F;
			m_fastHangTime = 0.1F;
			m_decay = float(1.0 - ::exp(-2.0 / m_samprate));
			m_oneMDecay = 1.0F - m_decay;
			break;

		case agcMED:
			m_hangTime = 0.25F;
			m_fastHangTime = 0.1F;
			m_decay = float(1.0 - ::exp(-4.0 / m_samprate));
			m_oneMDecay = 1.0F - m_decay;
			break;

		case agcFAST:
			m_hangTime = 0.1F;
			m_fastHangTime = 0.1F;
			m_decay = float(1.0 - ::exp(-10.0 / m_samprate));
			m_oneMDecay = 1.0F - m_decay;
			break;

		case agcLONG:
			m_hangTime = 0.75F;
			m_fastHangTime = 0.1F;
			m_decay = float(1.0 - ::exp(-0.5 / m_samprate));
			m_oneMDecay = 1.0F - m_decay;
			break;
    }
}

void CAGC::setGain(float gain)
{
	m_gainNow = gain;
}

void CAGC::setHangTime(float time)
{
	m_hangTime = time * 0.001F;
}

void CAGC::setGainTop(float gain)
{
	m_gainTop = gain;
}

void CAGC::setGainBottom(float gain)
{
	m_gainBottom = gain;
}

void CAGC::setAttack(float attack)
{
	m_attack = float(1.0 - ::exp(-1000.0 / (attack * m_samprate)));
	m_oneMAttack = (float)::exp (-1000.0 / (attack * m_samprate));

	m_sndex = (m_index + (int)(0.003 * m_samprate * attack)) & m_mask;
	m_fastIndex = (m_sndex + FASTLEAD * m_mask) & m_mask;

	m_fastHangTime = 0.1F;
}

void CAGC::setDecay(float decay)
{
	m_decay = float(1.0 - ::exp(-1000.0 / (decay * m_samprate)));
	m_oneMDecay = (float)::exp(-1000.0 / (decay * m_samprate));
}
