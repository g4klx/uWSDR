/* LMS.cpp

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

#include "LMS.h"
#include "Utils.h"

CLMS::CLMS(CXB* signal, unsigned int delay, float adaptionRate, float leakage, unsigned int adaptiveFilterSize, unsigned int filterType) :
m_signal(signal),
m_delay(delay),
m_adaptationRate(adaptionRate),
m_leakage(leakage),
m_adaptiveFilterSize(adaptiveFilterSize),
m_filterType(filterType),
m_delayLine(NULL),
m_adaptiveFilter(NULL),
m_delayLinePtr(0),
m_size(512),
m_mask(511)
{
	wxASSERT(signal != NULL);
	wxASSERT(filterType == LMS_INTERFERENCE || filterType == LMS_NOISE);

	m_delayLine      = new float[m_size];
	m_adaptiveFilter = new float[128];

	::memset(m_delayLine,      0x00, m_size * sizeof(float));
	::memset(m_adaptiveFilter, 0x00, 128 * sizeof(float));
}

CLMS::~CLMS()
{
	delete[] m_delayLine;
	delete[] m_adaptiveFilter;
}

void CLMS::setAdaptationRate(float adaptationRate)
{
	m_adaptationRate = adaptationRate;
}

void CLMS::setAdaptiveFilterSize(unsigned int adaptiveFilterSize)
{
	m_adaptiveFilterSize = adaptiveFilterSize;
}

void CLMS::setDelay(unsigned int delay)
{
	m_delay = delay;
}

void CLMS::setLeakage(float leakage)
{
	m_leakage = leakage;
}

void CLMS::process()
{
	switch (m_filterType) {

		case LMS_NOISE:
			processNoise();
			break;

		case LMS_INTERFERENCE:
			processInterference();
			break;

		default:
#if defined(__WXMSW__) || defined(__WXGTK__) || defined(__WXMAC__)
			wxLogError(wxT("Unknown filter type in LMS = %d"), m_filterType);
#elif defined(WIN32)
			// No WIN32 logging yet
#else
			::syslog(LOG_ERR, "Unknown filter type in LMS = %d", m_filterType);
#endif
			break;
	}
}

void CLMS::processInterference()
{
	float scl1 = 1.0F - m_adaptationRate * m_leakage;

	unsigned int n = CXBhave(m_signal);

	for (unsigned int i = 0; i < n; i++) {
		m_delayLine[m_delayLinePtr] = CXBreal(m_signal, i);
		float accum = 0.0F;
		float sum_sq = 0.0F;

		unsigned int j;
		for (j = 0; j < m_adaptiveFilterSize; j++) {
			unsigned int k = (j + m_delay + m_delayLinePtr) & m_mask;
			sum_sq += m_delayLine[k] * m_delayLine[k];
			accum += m_adaptiveFilter[j] * m_delayLine[k];
		}

		float error = CXBreal(m_signal, i) - accum;

		CXBreal(m_signal, i) = error;
		CXBimag(m_signal, i) = error;

		float scl2 = m_adaptationRate / (sum_sq + 1e-10);
		error *= scl2;

		for (j = 0; j < m_adaptiveFilterSize; j++) {
			unsigned int k = (j + m_delay + m_delayLinePtr) & m_mask;
			m_adaptiveFilter[j] = m_adaptiveFilter[j] * scl1 + error * m_delayLine[k];
		}

		m_delayLinePtr = (m_delayLinePtr + m_mask) & m_mask;
	}
}

void CLMS::processNoise()
{
	float scl1 = 1.0F - m_adaptationRate * m_leakage;

	unsigned int n = CXBhave(m_signal);

	for (unsigned int i = 0; i < n; i++) {
		m_delayLine[m_delayLinePtr] = CXBreal(m_signal, i);
		float accum = 0.0F;
		float sum_sq = 0.0F;

		unsigned int j;
		for (j = 0; j < m_adaptiveFilterSize; j++) {
			unsigned int k = (j + m_delay + m_delayLinePtr) & m_mask;
			sum_sq += m_delayLine[k] * m_delayLine[k];
			accum += m_adaptiveFilter[j] * m_delayLine[k];
		}

		float error = CXBreal(m_signal, i) - accum;

		CXBreal(m_signal, i) = accum;
		CXBimag(m_signal, i) = accum;

		float scl2 = m_adaptationRate / (sum_sq + 1e-10);
		error *= scl2;

		for (j = 0; j < m_adaptiveFilterSize; j++) {
			unsigned int k = (j + m_delay + m_delayLinePtr) & m_mask;
			m_adaptiveFilter[j] = m_adaptiveFilter[j] * scl1 + error * m_delayLine[k];
		}

		m_delayLinePtr = (m_delayLinePtr + m_mask) & m_mask;
	}
}
