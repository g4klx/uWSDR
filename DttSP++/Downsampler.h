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

#ifndef	Downsampler_H
#define	Downsampler_H

#include "Complex.h"

class CDownsampler {
public:
	CDownsampler(float fromSampleRate, float toSampleRate);
	~CDownsampler();

	unsigned int process(const float* in, unsigned int len, float* out);

private:
	COMPLEX*     m_taps;
	unsigned int m_n;
	unsigned int m_count;
	float*       m_buffer;
	unsigned int m_pos;

	float calculate() const;
};

#endif
