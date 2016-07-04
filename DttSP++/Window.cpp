/* Window.cpp

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2006-2008 by Jonathan Naylor, G4KLX

Implemented from code by Bill Schottstaedt of Snd Editor at CCRMA

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

#include <cstring>
#include <cmath>

#include "Window.h"
#include "Utils.h"

#if !defined(M_PI)
const double M_PI = 3.14159265358979323846;
#endif


/* shamelessly stolen from Bill Schottstaedt's clm.c */
/* made worse in the process, but enough for our purposes here */

/* mostly taken from
 *    Fredric J. Harris, "On the Use of Windows for Harmonic Analysis with the
 *    Discrete Fourier Transform," Proceedings of the IEEE, Vol. 66, No. 1,
 *    January 1978.
 *    Albert H. Nuttall, "Some Windows with Very Good Sidelobe Behaviour",
 *    IEEE Transactions of Acoustics, Speech, and Signal Processing, Vol. ASSP-29,
 *    No. 1, February 1981, pp 84-91
 *
 * JOS had slightly different numbers for the Blackman-Harris windows.
 */

float* CWindow::create(Windowtype type, unsigned int size, float* window)
{
	unsigned int i, j;
	float sr1;

	unsigned int midn = size >> 1;
	unsigned int midp1 = (size + 1) / 2;
	unsigned int midm1 = (size - 1) / 2;

	float freq = float(2.0 * M_PI / float(size));
	float rate = 1.0F / float(midn);
	float angle = 0.0F;
	float expn = float(::log(2.0) / float(midn + 1));
	float expsum = 1.0F;

	if (window == NULL)
		window = new float[size];
	::memset(window, 0x00, size * sizeof(float));

	switch (type) {
		case RECTANGULAR_WINDOW:
			for (i = 0; i < size; i++)
				window[i] = 1.0F;
			break;

		case HANN_WINDOW:
			for (i = 0, j = size - 1, angle = 0.0; i <= midn; i++, j--, angle += freq)
				window[j] = window[i] = float(0.5 - 0.5 * ::cos(angle));
			break;

		case WELCH_WINDOW:
			for (i = 0, j = size - 1; i <= midn; i++, j--) {
				float n = float(i - midm1) / float(midp1);
				window[j] = window[i] = 1.0F - n * n;
			}
			break;

		case PARZEN_WINDOW:
			for (i = 0, j = size - 1; i <= midn; i++, j--)
				window[j] = window[i] = float(1.0 - ::fabs(float(i - midm1) / float(midp1)));
			break;

		case BARTLETT_WINDOW:
			for (i = 0, j = size - 1, angle = 0.0; i <= midn; i++, j--, angle += rate)
				window[j] = window[i] = angle;
			break;

		case HAMMING_WINDOW:
			for (i = 0, j = size - 1, angle = 0.0; i <= midn; i++, j--, angle += freq)
				window[j] = window[i] = float(0.54 - 0.46 * ::cos(angle));
			break;

		case BLACKMAN2_WINDOW:	/* using Chebyshev polynomial equivalents here */
			for (i = 0, j = size - 1, angle = 0.0; i <= midn; i++, j--, angle += freq) {
				float cx = (float)::cos(angle);
				window[j] = window[i] = float(0.34401 + (cx * (-0.49755 + (cx * 0.15844))));
			}
			break;

		case BLACKMAN3_WINDOW:
			for (i = 0, j = size - 1, angle = 0.0; i <= midn; i++, j--, angle += freq) {
				float cx = (float)::cos(angle);
				window[j] = window[i] = float(0.21747 + (cx * (-0.45325 + (cx * (0.28256 - (cx * 0.04672))))));
			}
			break;

	    case BLACKMAN4_WINDOW:
			for (i = 0, j = size - 1, angle = 0.0; i <= midn; i++, j--, angle += freq) {
				float cx = (float)::cos(angle);
				window[j] = window[i] = float(0.084037 + (cx * (-0.29145 + (cx * (0.375696 + (cx * (-0.20762 + (cx * 0.041194))))))));
			}
			break;

		case EXPONENTIAL_WINDOW:
			for (i = 0, j = size - 1; i <= midn; i++, j--) {
				window[j] = window[i] = float(expsum - 1.0);
				expsum *= expn;
			}
			break;

		case RIEMANN_WINDOW:
			sr1 = float(2.0 * M_PI / float(size));
			for (i = 0, j = size - 1; i <= midn; i++, j--) {
				if (i == midn) {
					window[j] = window[i] = 1.0F;
				} else {
					float cx = sr1 * float(midn - i);
					window[j] = window[i] = float(::sin(cx) / cx);
				}
			}
			break;

		case BLACKMANHARRIS_WINDOW: {
				const float a0 = 0.35875F;
				const float a1 = 0.48829F;
				const float a2 = 0.14128F;
				const float a3 = 0.01168F;

				for (i = 0; i < size; i++)
					window[i] = float(a0 - a1 * (float)::cos(      2.0 * M_PI * float(i + 0.5F) / float(size - 1)) +
										   a2 * (float)::cos(2.0 * 2.0 * M_PI * float(i + 0.5F) / float(size - 1)) -
										   a3 * (float)::cos(3.0 * 2.0 * M_PI * float(i + 0.5F) / float(size - 1)));
			}
			break;

		case NUTTALL_WINDOW: {
				const float a0 = 0.3635819F;
				const float a1 = 0.4891775F;
				const float a2 = 0.1365995F;
				const float a3 = 0.0106411F;

				for (i = 0; i < size; i++)
					window[i] = float(a0 - a1 * (float)::cos(      2.0 * M_PI * float(i + 0.5) / float(size - 1)) +
										   a2 * (float)::cos(2.0 * 2.0 * M_PI * float(i + 0.5) / float(size - 1)) -
										   a3 * (float)::cos(3.0 * 2.0 * M_PI * float(i + 0.5) / float(size - 1)));
			}
			break;

		default:
			delete[] window;
			return NULL;
	}

	return window;
}
