/* FilterOVSV.cpp

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

#include "FilterOVSV.h"
#include "FIR.h"
#include "CXOps.h"
#include "Utils.h"
#include "CXB.h"

CFilterOVSV::CFilterOVSV(unsigned int bufLen, unsigned int pbits, float sampleRate, float lowFreq, float highFreq) :
m_pbits(pbits),
m_samprate(sampleRate),
m_bufLen(bufLen),
m_zfvec(NULL),
m_zivec(NULL),
m_zovec(NULL),
m_zrvec(NULL),
m_pfwd(),
m_pinv(),
m_scale(0.0F)
{
	wxASSERT(sampleRate > 0.0F);

	unsigned int fftLen = 2 * m_bufLen;

	m_zrvec = (COMPLEX*)::fftwf_malloc(fftLen * sizeof(COMPLEX));
	m_zfvec = (COMPLEX*)::fftwf_malloc(fftLen * sizeof(COMPLEX));
	m_zivec = (COMPLEX*)::fftwf_malloc(fftLen * sizeof(COMPLEX));
	m_zovec = (COMPLEX*)::fftwf_malloc(fftLen * sizeof(COMPLEX));

	wxASSERT(m_zrvec != NULL);
	wxASSERT(m_zfvec != NULL);
	wxASSERT(m_zivec != NULL);
	wxASSERT(m_zovec != NULL);

	::memset(m_zrvec, 0x00, fftLen * sizeof(COMPLEX));
	::memset(m_zfvec, 0x00, fftLen * sizeof(COMPLEX));
	::memset(m_zivec, 0x00, fftLen * sizeof(COMPLEX));
	::memset(m_zovec, 0x00, fftLen * sizeof(COMPLEX));

	// Prepare transforms for signal
	m_pfwd = ::fftwf_plan_dft_1d(fftLen, (fftwf_complex *)m_zrvec, (fftwf_complex *)m_zivec, FFTW_FORWARD, m_pbits);
	m_pinv = ::fftwf_plan_dft_1d(fftLen, (fftwf_complex *)m_zivec, (fftwf_complex *)m_zovec, FFTW_BACKWARD, m_pbits);

	m_scale = 1.0F / float(fftLen);

	setFilter(lowFreq, highFreq);
}

CFilterOVSV::~CFilterOVSV()
{
	::fftwf_free(m_zfvec);
	::fftwf_free(m_zivec);
	::fftwf_free(m_zovec);
	::fftwf_free(m_zrvec);

	::fftwf_destroy_plan(m_pfwd);
	::fftwf_destroy_plan(m_pinv);
}

void CFilterOVSV::setFilter(float lowFreq, float highFreq)
{
	wxASSERT(::fabs(lowFreq) < 0.5 * m_samprate);
	wxASSERT(::fabs(highFreq) < 0.5 * m_samprate);
	wxASSERT(::fabs(highFreq - lowFreq) >= 10.0F);

	unsigned int fftLen = 2 * m_bufLen;
	unsigned int ncoef  =  m_bufLen + 1;

	COMPLEX* coefs = CFIR::bandpass(lowFreq, highFreq, m_samprate, ncoef);

	COMPLEX* zcvec = (COMPLEX*)::fftwf_malloc(fftLen * sizeof(COMPLEX));
	wxASSERT(zcvec != NULL);
	::memset(zcvec, 0x00, fftLen * sizeof(COMPLEX));

	fftwf_plan ptmp = ::fftwf_plan_dft_1d(fftLen, (fftwf_complex *)zcvec, (fftwf_complex *)m_zfvec, FFTW_FORWARD, m_pbits);

#ifdef LHS
	for (unsigned int i = 0; i < ncoef; i++)
		zcvec[i] = coefs[i];
#else
	for (unsigned int i = 0; i < ncoef; i++)
		zcvec[fftLen - ncoef + i] = coefs[i];
#endif

	::fftwf_execute(ptmp);

	::fftwf_destroy_plan(ptmp);
	::fftwf_free(zcvec);

	delete[] coefs;

	normalize_vec_COMPLEX(m_zfvec, fftLen);
}

void CFilterOVSV::filter()
{
	unsigned int i, j;

	unsigned int fftLen = 2 * m_bufLen;

	// Input signal -> Z
	::fftwf_execute(m_pfwd);

	// Convolve in z
	for (i = 0; i < fftLen; i++)
		m_zivec[i] = Cmul(m_zivec[i], m_zfvec[i]);

	// Z convolved signal -> time output signal
	::fftwf_execute(m_pinv);

	// Scale
	for (i = 0; i < m_bufLen; i++) {
		m_zovec[i].re *= m_scale;
		m_zovec[i].im *= m_scale;
	}

	// Prepare input signal vector for next fill
	for (i = 0, j = m_bufLen; i < m_bufLen; i++, j++)
		m_zrvec[i] = m_zrvec[j];
}

void CFilterOVSV::reset()
{
	::memset(m_zrvec, 0x00, 2 * m_bufLen * sizeof(COMPLEX));
}

// Where to put next batch of samples to filter
COMPLEX* CFilterOVSV::fetchPoint()
{
	return m_zrvec + m_bufLen;
}

// How many samples to put there
unsigned int CFilterOVSV::fetchSize()
{
	return m_bufLen;
}

// Where samples should be taken from after filtering
#ifdef LHS
COMPLEX* CFilterOVSV::storePoint()
{
	return m_zovec + m_buflen;
}
#else
COMPLEX* CFilterOVSV::storePoint()
{
	return m_zovec;
}
#endif

// How many samples to take
/* NB strategy. This is the number of good samples in the
   left half of the true buffer. Samples in right half
   are circular artifacts and are ignored. */
unsigned int CFilterOVSV::storeSize()
{
	return m_bufLen;
}

COMPLEX* CFilterOVSV::getZFvec()
{
	return m_zfvec;
}
