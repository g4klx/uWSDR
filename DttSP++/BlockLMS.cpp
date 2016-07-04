/* BlockLMS.cpp

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

#include "BlockLMS.h"
#include "Utils.h"

const float BLKSCL = 1.0F / 256.0F;

CBlockLMS::CBlockLMS(CXB* signal, float adaptionRate, float leakRate, unsigned int filterType, unsigned int pbits) :
m_signal(signal),
m_adaptationRate(adaptionRate),
m_leakRate(1.0F - leakRate),
m_filterType(filterType),
m_delayLine(NULL),
m_y(NULL),
m_wHat(NULL),
m_xHat(NULL),
m_yHat(NULL),
m_error(NULL),
m_errHat(NULL),
m_update1(NULL),
m_update2(NULL),
m_wPlan(NULL),
m_xPlan(NULL),
m_yPlan(NULL),
m_errHatPlan(NULL),
m_updPlan(NULL)
{
	wxASSERT(signal != NULL);
	wxASSERT(filterType == BLMS_INTERFERENCE || filterType == BLMS_NOISE);
	wxASSERT(pbits > 0);

	m_delayLine = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_y         = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_wHat      = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_xHat      = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_yHat      = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_error     = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_errHat    = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_update1   = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));
	m_update2   = (COMPLEX*)::fftwf_malloc(256 * sizeof(COMPLEX));

	wxASSERT(m_delayLine != NULL);
	wxASSERT(m_y != NULL);
	wxASSERT(m_wHat != NULL);
	wxASSERT(m_xHat != NULL);
	wxASSERT(m_yHat != NULL);
	wxASSERT(m_error != NULL);
	wxASSERT(m_errHat != NULL);
	wxASSERT(m_update1 != NULL);
	wxASSERT(m_update2 != NULL);

	::memset(m_delayLine, 0x00, 256 * sizeof(COMPLEX));
	::memset(m_y,         0x00, 256 * sizeof(COMPLEX));
	::memset(m_wHat,      0x00, 256 * sizeof(COMPLEX));
	::memset(m_xHat,      0x00, 256 * sizeof(COMPLEX));
	::memset(m_yHat,      0x00, 256 * sizeof(COMPLEX));
	::memset(m_error,     0x00, 256 * sizeof(COMPLEX));
	::memset(m_errHat,    0x00, 256 * sizeof(COMPLEX));
	::memset(m_update1,   0x00, 256 * sizeof(COMPLEX));
	::memset(m_update2,   0x00, 256 * sizeof(COMPLEX));

	m_wPlan      = ::fftwf_plan_dft_1d(256, (fftwf_complex *)m_update1,   (fftwf_complex *)m_update2,   FFTW_FORWARD,  pbits);
	m_xPlan      = ::fftwf_plan_dft_1d(256, (fftwf_complex *)m_delayLine, (fftwf_complex *)m_xHat,      FFTW_FORWARD,  pbits);
	m_yPlan      = ::fftwf_plan_dft_1d(256, (fftwf_complex *)m_yHat,      (fftwf_complex *)m_y,         FFTW_BACKWARD, pbits);
	m_errHatPlan = ::fftwf_plan_dft_1d(256, (fftwf_complex *)m_error,     (fftwf_complex *)m_errHat,    FFTW_FORWARD,  pbits);
	m_updPlan    = ::fftwf_plan_dft_1d(256, (fftwf_complex *)m_errHat,    (fftwf_complex *)m_update1,   FFTW_BACKWARD, pbits);
}

CBlockLMS::~CBlockLMS()
{
	::fftwf_destroy_plan(m_wPlan);
	::fftwf_destroy_plan(m_xPlan);
	::fftwf_destroy_plan(m_yPlan);
	::fftwf_destroy_plan(m_errHatPlan);
	::fftwf_destroy_plan(m_updPlan);

	::fftwf_free(m_update1);
	::fftwf_free(m_update2);
	::fftwf_free(m_wHat);
	::fftwf_free(m_xHat);
	::fftwf_free(m_yHat);
	::fftwf_free(m_error);
	::fftwf_free(m_errHat);
	::fftwf_free(m_y);
	::fftwf_free(m_delayLine);
}

void CBlockLMS::setAdaptationRate(float adaptationRate)
{
	m_adaptationRate = adaptationRate;
}

void CBlockLMS::process()
{
	unsigned int sigsize = CXBhave(m_signal);
	unsigned int sigidx = 0;

	do {
		::memcpy(m_delayLine,       &m_delayLine[128],          sizeof(COMPLEX) * 128);	// do overlap move
		::memcpy(&m_delayLine[128], &CXBdata(m_signal, sigidx), sizeof(COMPLEX) * 128);	// copy in new data

		::fftwf_execute(m_xPlan);	// compute transform of input data

		unsigned int j;
		for (j = 0; j < 256; j++) {
			m_yHat[j] = Cmul(m_wHat[j], m_xHat[j]);	// Filter new signal in freq. domain
			m_xHat[j] = Conjg(m_xHat[j]);			// take input data's complex conjugate
		}

		::fftwf_execute(m_yPlan);	//compute output signal transform

		for (j = 128; j < 256; j++)
			m_y[j] = Cscl(m_y[j], BLKSCL);

		::memset(m_y, 0x00, 128 * sizeof (COMPLEX));

		for (j = 128; j < 256; j++)
			m_error[j] = Csub(m_delayLine[j], m_y[j]);	// compute error signal

		switch (m_filterType) {
			case BLMS_INTERFERENCE:
				::memcpy(&CXBdata(m_signal, sigidx), &m_error[128], 128 * sizeof(COMPLEX));	// if notch filter, output error
				break;
			case BLMS_NOISE:
				::memcpy(&CXBdata(m_signal, sigidx), &m_y[128],     128 * sizeof(COMPLEX));	// if noise filter, output y
				break;
			default:
#if defined(__WXMSW__) || defined(__WXGTK__) || defined(__WXMAC__)
				wxLogError(wxT("Unknown filter type in Block LMS = %d"), m_filterType);
#elif defined(WIN32)
				// No WIN32 logging yet
#else
				::syslog(LOG_ERR, "Unknown filter type in Block LMS = %d", m_filterType);
#endif
				break;
		}

		::fftwf_execute(m_errHatPlan);	// compute transform of the error signal

		for (j = 0; j < 256; j++)
			m_errHat[j] = Cmul(m_errHat[j], m_xHat[j]);	// compute cross correlation transform

		::fftwf_execute(m_updPlan);	// compute inverse transform of cross correlation transform

		for (j = 0; j < 128; j++)
			m_update1[j] = Cscl(m_update1[j], BLKSCL);

		::memset(&m_update1[128], 0x00, sizeof (COMPLEX) * 128);	// zero the last block of the update, so we get

		// filter coefficients only at front of buffer
		::fftwf_execute(m_wPlan);

		for (j = 0; j < 256; j++) {
			m_wHat[j] = Cadd(Cscl(m_wHat[j], m_leakRate),	// leak the W away
			Cscl(m_update2[j], m_adaptationRate));			// update at adaptation rate
		}

		sigidx += 128;		// move to next block in the signal buffer
	} while (sigidx < sigsize);	// done?
}
