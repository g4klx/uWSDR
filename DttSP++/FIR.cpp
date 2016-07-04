/* FIR.cpp

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2006-2008 by Jonathan Naylor, G4KLX

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

#include <cmath>

#include "FIR.h"
#include "CXB.h"
#include "Window.h"

#if !defined(M_PI)
const double M_PI = 3.14159265358979323846;
#endif

COMPLEX* CFIR::lowpass(float cutoff, float samprate, unsigned int size)
{
	if (cutoff < 0.0F || cutoff > (samprate / 2.0F))
		return NULL;
	if (size < 1)
		return NULL;

	float fc = cutoff / samprate;

	if (!(size & 01))
		size++;

	int midpoint = (size >> 01) | 01;

	COMPLEX* h = new COMPLEX[size];

	float* w = CWindow::create(BLACKMANHARRIS_WINDOW, size);

	for (int i = 1; i <= int(size); i++) {
		int  j = i - 1;
		float k = float(i - midpoint);

		if (i != midpoint) {
			h[j].re = float((::sin(2.0 * M_PI * k * fc) / (M_PI * k)) * w[j]);
			h[j].im = 0.0F;
		} else {
			h[midpoint - 1].re = 2.0F * fc;
			h[midpoint - 1].im = 0.0F;
		}
	}

	delete[] w;

	return h;
}

COMPLEX* CFIR::bandpass(float lo, float hi, float samprate, unsigned int size)
{
	if (lo < -(samprate / 2.0F) || hi > (samprate / 2.0F) || hi <= lo)
		return NULL;
	if (size < 1)
		return NULL;

	if (!(size & 01))
		size++;

	int midpoint = (size >> 01) | 01;

	COMPLEX* h = new COMPLEX[size];

	float* w = CWindow::create(BLACKMANHARRIS_WINDOW, size);

	lo /= samprate;
	hi /= samprate;

	float fc = float((hi - lo) / 2.0F);
	float ff = float((lo + hi) * M_PI);

	for (int i = 1; i <= int(size); i++) {
		int j = i - 1;
		float k = float(i - midpoint);
		float phs = ff * k;

		float tmp;
		if (i != midpoint)
			tmp = float((::sin(2.0 * M_PI * k * fc) / (M_PI * k)) * w[j]);
		else
			tmp = 2.0F * fc;

		tmp *= 2.0F;

		h[j].re = float(tmp * ::cos(phs));
		h[j].im = float(tmp * ::sin(phs));
	}

	delete[] w;

	return h;
}

COMPLEX* CFIR::highpass(float cutoff, float samprate, unsigned int size)
{
	if (cutoff < 0.0F || cutoff > (samprate / 2.0F))
		return NULL;
	if (size < 1)
		return NULL;

	if (!(size & 01))
		size++;

	int midpoint = (size >> 01) | 01;

	COMPLEX* h = new COMPLEX[size];

	float* w = CWindow::create(BLACKMANHARRIS_WINDOW, size);

	float fc = cutoff / samprate;

	int i;
	for (i = 1; i <= int(size); i++) {
		int j = i - 1;
		float k = float(i - midpoint);

		if (i != midpoint) {
			h[j].re = float((::sin(2.0 * M_PI * k * fc) / (M_PI * k)) * w[j]);
			h[j].im = 0.0F;
		} else {
			h[midpoint - 1].re = 2.0F * fc;
			h[midpoint - 1].im = 0.0F;
		}
	}

	delete[] w;

	for (i = 1; i <= int(size); i++) {
		int j = i - 1;

		if (i != midpoint)
			h[j].re = -h[j].re;
		else
			h[midpoint - 1].re = 1.0F - h[midpoint - 1].re;
	}

	return h;
}

COMPLEX* CFIR::hilbert(float lo, float hi, float samprate, unsigned int size)
{
	if (lo < 0.0F || hi > (samprate / 2.0F) || hi <= lo)
		return NULL;
	if (size < 1)
		return NULL;

	if (!(size & 01))
		size++;

	int midpoint = (size >> 01) | 01;

	COMPLEX* h = new COMPLEX[size];

	float* w = CWindow::create(BLACKMANHARRIS_WINDOW, size);

	lo /= samprate;
	hi /= samprate;

	float fc = float((hi - lo) / 2.0F);
	float ff = float((lo + hi) * M_PI);

	for (int i = 1; i <= int(size); i++) {
		int j = i - 1;
		float k = float(i - midpoint);
		float phs = ff * k;

		float tmp;
		if (i != midpoint)
			tmp = float((::sin(2.0 * M_PI * k * fc) / (M_PI * k)) * w[j]);
		else
			tmp = 2.0F * fc;

		tmp *= 2.0F;

		h[j].re = 0.0F;
		h[j].im = float(tmp * ::sin(phs));
	}

	delete[] w;

	return h;
}

COMPLEX* CFIR::bandstop(float lo, float hi, float samprate, unsigned int size)
{
	if (lo < 0.0F || hi > (samprate / 2.0F) || hi <= lo)
		return NULL;
	if (size < 1)
		return NULL;

	if (!(size & 01))
		size++;

	int midpoint = (size >> 01) | 01;
	COMPLEX* h = new COMPLEX[size];

	float* w = CWindow::create(BLACKMANHARRIS_WINDOW, size);

	lo /= samprate;
	hi /= samprate;

	float fc = float((hi - lo) / 2.0F);
	float ff = float((lo + hi) * M_PI);

	int i;
	for (i = 1; i <= int(size); i++) {
		int j = i - 1;
		float k = float(i - midpoint);
		float phs = ff * k;

		float tmp;
		if (i != midpoint)
			tmp = float((::sin(2.0 * M_PI * k * fc) / (M_PI * k)) * w[j]);
		else
			tmp = 2.0F * fc;

		tmp *= 2.0F;

		h[j].re = float(tmp * ::cos(phs));
		h[j].im = float(tmp * ::sin(phs));
	}

	delete[] w;

	for (i = 1; i <= int(size); i++) {
		int j = i - 1;

		if (i != midpoint)
			h[j] = Cmul(h[j], cxminusone);
		else
			h[midpoint - 1] = Csub(cxone, h[midpoint - 1]);
	}

	return h;
}
