/* Equaliser.cpp

This file is part of a program that implements a Software-Defined Radio.

Copyright (C) 2004, 2005, 2006 by Frank Brickle, AB2KT and Bob McGwier, N4HY
Copyright (C) 2013 by Jonathan Naylor, G4KLX

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

#include "Equaliser.h"
#include "Complex.h"
#include "FIR.h"

#include "fftw3.h"

CEqualiser::CEqualiser(float samprate, unsigned int pbits, CXB* vec) :
m_samprate(samprate),
m_pbits(pbits),
m_filter(NULL),
m_buf(vec),
m_in(NULL),
m_out(NULL),
m_zfvec(NULL)
{
	wxASSERT(samprate > 0.0F);
	wxASSERT(vec != NULL);

	m_filter = new CFilterOVSV(257U, pbits, samprate, -6000.0F, 6000.0F);

	m_in  = m_filter->fetchPoint();
	m_out = m_filter->storePoint();

	m_zfvec = m_filter->getZFvec();
}

CEqualiser::~CEqualiser()
{
	delete m_filter;
}

void CEqualiser::setValues(unsigned int n, const int* vals)
{
	wxASSERT(n == 4U);
	wxASSERT(vals != NULL);

	COMPLEX* filtcoef = (COMPLEX*)::fftwf_malloc(512U * sizeof(COMPLEX));
	COMPLEX* tmpcoef  = (COMPLEX*)::fftwf_malloc(257U * sizeof(COMPLEX));

	wxASSERT(filtcoef != NULL);
	wxASSERT(tmpcoef != NULL);

	::memset(filtcoef, 0x00, 512U * sizeof(COMPLEX));
	::memset(tmpcoef,  0x00, 257U * sizeof(COMPLEX));

	float preamp = ::pow(10.0F, float(vals[0U]) / 20.0F) * 0.5F;

	float gain0 = ::pow(10.0F, float(vals[1U]) / 20.0F) * preamp;
	float gain1 = ::pow(10.0F, float(vals[2U]) / 20.0F) * preamp;
	float gain2 = ::pow(10.0F, float(vals[3U]) / 20.0F) * preamp;

	COMPLEX* coefs = CFIR::bandpass(-400.0F, 400.0F, m_samprate, 257U);
	for (unsigned int i = 0U; i < 257U; i++)
		tmpcoef[i] = Cscl(coefs[i], gain0);
	delete[] coefs;

	coefs = CFIR::bandpass(400.0F, 1500.0F, m_samprate, 257U);
	for (unsigned int i = 0U; i < 257U; i++)
		tmpcoef[i] = Cadd(tmpcoef[i], Cscl(coefs[i], gain1));
	delete[] coefs;

	coefs = CFIR::bandpass(-1500.0F, -400.0F, m_samprate, 257U);
	for (unsigned int i = 0U; i < 257U; i++)
		tmpcoef[i] = Cadd(tmpcoef[i], Cscl(coefs[i], gain1));
	delete[] coefs;

	coefs = CFIR::bandpass(1500.0F, 6000.0F, m_samprate, 257U);
	for (unsigned int i = 0U; i < 257U; i++)
		tmpcoef[i] = Cadd(tmpcoef[i], Cscl(coefs[i], gain2));
	delete[] coefs;

	coefs = CFIR::bandpass(-6000.0F, -1500.0F, m_samprate, 257U);
	for (unsigned int i = 0U; i < 257U; i++)
		tmpcoef[i] = Cadd(tmpcoef[i], Cscl(coefs[i], gain2));
	delete[] coefs;

	for (unsigned int i = 0; i < 257U; i++)
		filtcoef[254U + i] = tmpcoef[i];

	fftwf_plan ptmp = ::fftwf_plan_dft_1d(512U, (fftwf_complex*)filtcoef, (fftwf_complex*)m_zfvec, FFTW_FORWARD, m_pbits);

	::fftwf_execute(ptmp);
	::fftwf_destroy_plan(ptmp);

	::fftwf_free(filtcoef);
	::fftwf_free(tmpcoef);
}

void CEqualiser::process()
{
	unsigned int sigsize = CXBhave(m_buf);

	unsigned int sigidx = 0U;
	do {
		::memcpy(m_in, &CXBdata(m_buf, sigidx), 256U * sizeof(COMPLEX));

		m_filter->filter();

		::memcpy(&CXBdata(m_buf, sigidx), m_out, 256U * sizeof(COMPLEX));

		sigidx += 256U;
	} while (sigidx < sigsize);
}
