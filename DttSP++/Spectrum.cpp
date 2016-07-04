/* Spectrum.cpp

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

#include <cmath>
#include <algorithm>
using std::max;
using std::min;

#include "Spectrum.h"
#include "Window.h"
#include "FIR.h"
#include "Utils.h"

CSpectrum::CSpectrum(unsigned int size, unsigned int planbits, SPECTRUMscale scale) :
m_accum(NULL),
m_timebuf(NULL),
m_freqbuf(NULL),
m_fill(0),
m_scale(scale),
m_size(size),
m_mask(size - 1),
m_winType(BLACKMANHARRIS_WINDOW),
m_window(NULL),
m_planbits(planbits),
m_plan(),
m_polyphase(false)
{
	wxASSERT(size > 0U);

	m_accum  = newCXB(m_size * 16, NULL);
	m_window = new float[m_size * 16];

	COMPLEX* timebuf = (COMPLEX*)::fftwf_malloc(m_size * sizeof(COMPLEX));
	COMPLEX* freqbuf = (COMPLEX*)::fftwf_malloc(m_size * sizeof(COMPLEX));

	wxASSERT(timebuf != NULL);
	wxASSERT(freqbuf != NULL);

	::memset(CXBbase(m_accum), 0x00, 16 * m_size * sizeof(COMPLEX));
	::memset(m_window, 0x00, 16 * m_size * sizeof(float));
	::memset(timebuf,  0x00, m_size * sizeof(COMPLEX));
	::memset(freqbuf,  0x00, m_size * sizeof(COMPLEX));

	m_timebuf = newCXB(m_size, timebuf);
	m_freqbuf = newCXB(m_size, freqbuf);

	CWindow::create(BLACKMANHARRIS_WINDOW, m_size, m_window);

	m_plan = ::fftwf_plan_dft_1d(m_size, (fftwf_complex *)CXBbase(m_timebuf), (fftwf_complex *)CXBbase(m_freqbuf), FFTW_FORWARD, m_planbits);
}

CSpectrum::~CSpectrum()
{
	::fftwf_free(CXBbase(m_timebuf));
	::fftwf_free(CXBbase(m_freqbuf));

	delCXB(m_accum);
	delCXB(m_timebuf);
	delCXB(m_freqbuf);

	delete[] m_window;

	::fftwf_destroy_plan(m_plan);
}

void CSpectrum::setScale(SPECTRUMscale scale)
{
	m_scale = scale;
}

void CSpectrum::setWindowType(Windowtype type)
{
	if (!m_polyphase)
		CWindow::create(type, m_size, m_window);

	m_winType = type;
}

void CSpectrum::setPolyphaseFlag(bool setit)
{
	if (m_polyphase != setit) {
		if (setit) {
			m_polyphase = true;
			m_mask = (8 * m_size) - 1;

			unsigned int i;

			COMPLEX* fir = CFIR::lowpass(1.0F, float(m_size), 8 * m_size - 1);
			::memset(m_window, 0x00, 8 * sizeof(float) * m_size);
			for (i = 0; i < 8 * m_size - 1; i++)
				m_window[i] = fir[i].re;
			delete[] fir;

			float maxTap = 0.0F;
			for (i = 0; i < 8 * m_size; i++)
				maxTap = ::max(maxTap, (float)::fabs(m_window[i]));

			maxTap = 1.0F / maxTap;

			for (i = 0; i < 8 * m_size; i++)
				m_window[i] *= maxTap;
		} else {
			m_polyphase = false;
			m_mask = m_size - 1;
			::memset(m_window, 0x00, sizeof(float) * m_size);
			CWindow::create(m_winType, m_size - 1, m_window);
		}

		reinitSpectrum();
	}
}

void CSpectrum::setData(CXB* buf)
{
	wxASSERT(buf != NULL);

	::memcpy(&CXBdata(m_accum, m_fill), CXBbase(buf), CXBhave(buf) * sizeof(COMPLEX));

	m_fill = (m_fill + CXBhave(buf)) & m_mask;
}

void CSpectrum::reinitSpectrum()
{
	size_t polysize = 1;
	m_fill = 0;

	if (m_polyphase)
		polysize = 8;

	::memset(CXBbase(m_accum), 0, polysize * m_size * sizeof(float));
}

// snapshot of current signal
void CSpectrum::snapSpectrum()
{
	// where most recent signal started
	unsigned int j = m_fill;

	// copy starting from there in circular fashion,
	// applying window as we go
	if (!m_polyphase) {
		for (unsigned int i = 0; i < m_size; i++) {
			CXBdata(m_timebuf, i) = Cscl(CXBdata(m_accum, j), m_window[i]);
			j = (j + 1) & m_mask;
		}
	} else {
		for (unsigned int i = 0; i < m_size; i++) {
			CXBreal(m_timebuf, i) = CXBreal(m_accum, j) * m_window[i];
			CXBimag(m_timebuf, i) = CXBimag(m_accum, j) * m_window[i];

			for (unsigned int k = 1; k < 8; k++) {
				unsigned int accumidx = (j + k * m_size) & m_mask;
				unsigned int winidx = i + k * m_size;

				CXBreal(m_timebuf, i) += CXBreal(m_accum, accumidx) * m_window[winidx];
				CXBimag(m_timebuf, i) += CXBimag(m_accum, accumidx) * m_window[winidx];
			}

			j = (j + 1) & m_mask;
		}
	}
}

void CSpectrum::snapScope()
{
	// where most recent signal started
	unsigned int j = m_fill;

	// copy starting from there in circular fashion
	for (unsigned int i = 0; i < m_size; i++) {
		CXBdata(m_timebuf, i) = CXBdata(m_accum, j);
		j = (j + 1) & m_mask;
	}
}

// snapshot -> frequency domain
void CSpectrum::computeSpectrum(float* spectrum)
{
	wxASSERT(spectrum != NULL);

	unsigned int half = m_size / 2;
	unsigned int i, j;

	// assume timebuf has windowed current snapshot
	::fftwf_execute(m_plan);

	if (m_scale == SPEC_MAG) {
		for (i = 0, j = half; i < half; i++, j++) {
			spectrum[i] = (float)Cmag(CXBdata(m_freqbuf, j));
			spectrum[j] = (float)Cmag(CXBdata(m_freqbuf, i));
		}
	} else {				// SPEC_PWR
		for (i = 0, j = half; i < half; i++, j++) {
			spectrum[i] = float(10.0 * ::log10(Csqrmag(CXBdata(m_freqbuf, j)) + 1e-60));
			spectrum[j] = float(10.0 * ::log10(Csqrmag(CXBdata(m_freqbuf, i)) + 1e-60));
		}
	}
}

void CSpectrum::computeScopeReal(float* results, unsigned int numpoints)
{
	wxASSERT(results != NULL);
	wxASSERT(numpoints <= m_size);

	::memset(results, 0x00, numpoints * sizeof(float));

	unsigned int len = numpoints;
	if (len > m_size)
		len = m_size;

	for (unsigned int i = 0; i < len; i++)
		results[i] = CXBreal(m_timebuf, i);
}

void CSpectrum::computeScopeComplex(float* results, unsigned int numpoints)
{
	wxASSERT(results != NULL);
	wxASSERT(numpoints <= m_size);

	::memset(results, 0x00, numpoints * sizeof(float));

	unsigned int len = numpoints / 2U;
	if (len > m_size)
		len = m_size;

	for (unsigned int i = 0; i < len; i++) {
		*results++ = CXBreal(m_timebuf, i);
		*results++ = CXBimag(m_timebuf, i);
	}
}

