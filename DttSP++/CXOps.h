/* CXOps.h

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

#ifndef _cxops_h
#define _cxops_h

#include <cmath>

#include "Complex.h"
#include "Utils.h"


// Useful constants
const COMPLEX cxzero     = { 0.0, 0.0 };
const COMPLEX cxone      = { 1.0, 0.0 };
const COMPLEX cxJ        = { 0.0, 1.0 };
const COMPLEX cxminusone = { -1.0, 0.0 };
const COMPLEX cxminusJ   = { 0.0, -1.0 };


// scalar
inline COMPLEX Cscl(COMPLEX x, float a)
{
	COMPLEX z;
	z.re = x.re * a;
	z.im = x.im * a;
	return z;
}

inline COMPLEX Cadd(COMPLEX x, COMPLEX y)
{
	COMPLEX z;
	z.re = x.re + y.re;
	z.im = x.im + y.im;
	return z;
}

inline COMPLEX Csub(COMPLEX x, COMPLEX y)
{
	COMPLEX z;
	z.re = x.re - y.re;
	z.im = x.im - y.im;
	return z;
}

inline COMPLEX Cmul(COMPLEX x, COMPLEX y)
{
	COMPLEX z;
	z.re = x.re * y.re - x.im * y.im;
	z.im = x.im * y.re + x.re * y.im;
	return z;
}

inline COMPLEX Cdiv(COMPLEX x, COMPLEX y)
{
	float d = y.re * y.re + y.im * y.im;

	COMPLEX z;
	z.re = (x.re * y.re + x.im * y.im) / d;
	z.im = (y.re * x.im - y.im * x.re) / d;
	return z;
}

inline float Cappmag(COMPLEX z)
{
	float tmpr = float(::fabs(z.re));
	float tmpi = float(::fabs(z.im));

	return (tmpr < tmpi) ? 0.4F * tmpr + 0.7F * tmpi : 0.4F * tmpi + 0.7F * tmpr;
}

inline float Cmag(COMPLEX z)
{
	return float(::sqrt(z.re * z.re + z.im * z.im));
}

inline float Cabs(COMPLEX z)
{
	return float(::sqrt(z.re * z.re + z.im * z.im));
}

inline float Csqrmag(COMPLEX z)
{
	return float(z.re * z.re + z.im * z.im);
}

inline COMPLEX Cmplx(float x, float y)
{
	COMPLEX z;
	z.re = x;
	z.im = y;
	return z;
}

inline COMPLEX Conjg(COMPLEX z)
{
	return Cmplx(z.re, -z.im);
}

inline COMPLEX Cexp(COMPLEX z)
{
	float r = float(::exp(z.re));

	return Cmplx(float(r * ::cos(z.im)), float(r * ::sin(z.im)));
}

#endif
