/*
 *   Copyright (C) 2006-2008,2013 by Jonathan Naylor G4KLX
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

#include "SignalReader.h"

#include <wx/debug.h>
#include <wx/log.h>


CSignalReader::CSignalReader(float frequency, float noiseAmplitude, float signalAmplitude, float balanceErr, float amplitudeErr, bool swapIQ, IDataReader* reader) :
CThreadReader(reader),
m_frequency(frequency),
m_noiseAmplitude(noiseAmplitude),
m_signalAmplitude(signalAmplitude),
m_balanceErr(balanceErr),
m_amplitudeErr(amplitudeErr),
m_blockSize(0),
m_swapIQ(swapIQ),
m_callback(NULL),
m_id(0),
m_buffer(NULL),
m_awgn(NULL),
m_noiseSize(0),
m_cosVal(0.0F),
m_sinVal(0.0F),
m_cosDelta(0.0F),
m_sinDelta(0.0F),
m_awgnN(0)
{
	wxASSERT(m_noiseAmplitude >= 0.0 && m_noiseAmplitude < 1.0);
	wxASSERT(m_signalAmplitude >= 0.0 && m_signalAmplitude < 1.0);
	wxASSERT((m_signalAmplitude + m_noiseAmplitude) < 1.0);
	wxASSERT(m_frequency > 0.0F);
}

CSignalReader::~CSignalReader()
{
	delete[] m_buffer;
	delete[] m_awgn;
}

void CSignalReader::setCallback(IDataCallback* callback, int id)
{
	m_callback = callback;
	m_id       = id;
}

bool CSignalReader::open(float sampleRate, unsigned int blockSize)
{
	wxASSERT(m_frequency < (sampleRate + 0.5F) / 2.0F);

	m_blockSize = blockSize;

	m_noiseSize = int(sampleRate + 0.5F) * 10 * 2;

	m_buffer = new float[m_blockSize * 2];
	m_awgn   = new float[m_noiseSize];

	// Skew the balance if needed
	m_cosVal = 1.0F;
	m_sinVal = ::sin(m_balanceErr * M_PI / 180.0F);

	float delta = m_frequency / sampleRate * 2.0 * M_PI;
	m_cosDelta = ::cos(delta);
	m_sinDelta = ::sin(delta);

	for (unsigned int i = 0; i < m_noiseSize / 2; i++) {
		float x1, x2, w;

		do {
			x1 = (2.0 * (float(::rand()) / float(RAND_MAX)) - 1.0);
			x2 = (2.0 * (float(::rand()) / float(RAND_MAX)) - 1.0);
			w = x1 * x1 + x2 * x2;
		} while (w >= 1.0);

		w = ::sqrt((-2.0 * ::log(w)) / w);

		m_awgn[i * 2 + 0] = x1 * w * m_noiseAmplitude;
		m_awgn[i * 2 + 1] = x2 * w * m_noiseAmplitude;
	}

	m_awgnN = 0;

	wxLogMessage(wxT("SignalReader: started"));

	return CThreadReader::open(sampleRate, blockSize);
}

bool CSignalReader::create()
{
	wxASSERT(m_callback != NULL);

	for (unsigned int i = 0; i < m_blockSize; i++) {
		float tmpVal = m_cosVal * m_cosDelta - m_sinVal * m_sinDelta;
		m_sinVal = m_cosVal * m_sinDelta + m_sinVal * m_cosDelta;
		m_cosVal = tmpVal;

		if (m_swapIQ) {
			m_buffer[i * 2 + 1] = m_awgn[m_awgnN++] + m_cosVal * m_signalAmplitude;
			m_buffer[i * 2 + 0] = m_awgn[m_awgnN++] + m_sinVal * m_signalAmplitude * (1.0F + m_amplitudeErr);
		} else {
			m_buffer[i * 2 + 0] = m_awgn[m_awgnN++] + m_cosVal * m_signalAmplitude;
			m_buffer[i * 2 + 1] = m_awgn[m_awgnN++] + m_sinVal * m_signalAmplitude * (1.0F + m_amplitudeErr);
		}

		if (m_awgnN >= m_noiseSize)
			m_awgnN = 0;
	}

	m_callback->callback(m_buffer, m_blockSize, m_id);

	return true;
}
