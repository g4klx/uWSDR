/*
 *   Copyright (C) 2013 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <wx/wx.h>

#include "Upsampler.h"
#include "FIR.h"

const unsigned int FIR_COEFFS_LEN = 50U;

CUpsampler::CUpsampler(float fromSampleRate, float toSampleRate) :
m_taps(NULL),
m_n(0U),
m_buffer(NULL),
m_pos(0U)
{
	float f = fromSampleRate * 0.4F;

	m_taps = CFIR::lowpass(f, toSampleRate, FIR_COEFFS_LEN);

	m_n = (unsigned int)(toSampleRate / fromSampleRate);

	m_buffer = new float[FIR_COEFFS_LEN];
	::memset(m_buffer, 0x00U, FIR_COEFFS_LEN * sizeof(float));
}

CUpsampler::~CUpsampler()
{
	delete[] m_taps;
	delete[] m_buffer;
}

unsigned int CUpsampler::process(const float *in, unsigned int len, float *out)
{
	wxASSERT(in != NULL);
	wxASSERT(out != NULL);

	float* p = out;

	for (unsigned int i = 0U; i < len; i++) {
		m_buffer[m_pos++] = in[i];
		if (m_pos >= FIR_COEFFS_LEN)
			m_pos = 0U;

		*p++ = calculate();

		for (unsigned int j = 0U; j < (m_n - 1U); j++) {
			m_buffer[m_pos++] = 0.0F;
			if (m_pos >= FIR_COEFFS_LEN)
				m_pos = 0U;

			*p++ = calculate();
		}
	}

	return p - out;
}

float CUpsampler::calculate() const
{
	float acc = 0.0F;

	unsigned int pos = m_pos;

	for (unsigned int i = 0U; i < FIR_COEFFS_LEN; i++) {
		acc += m_buffer[pos++] * m_taps[i].re;
		if (pos >= FIR_COEFFS_LEN)
			pos = 0U;
	}

	return acc;
}
