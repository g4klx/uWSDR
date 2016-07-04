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

#ifndef	SignalReader_H
#define	SignalReader_H

#include <wx/wx.h>

#include "ThreadReader.h"


class CSignalReader : public CThreadReader {
public:
	CSignalReader(float frequency, float noiseAmplitude, float signalAmplitude, float balanceErr, float amplitudeErr, bool swapIQ, IDataReader* reader = NULL);

	virtual void setCallback(IDataCallback* callback, int id);

	virtual bool open(float sampleRate, unsigned int blockSize);

	virtual bool create();

protected:
	virtual ~CSignalReader();

private:
	float          m_frequency;
	float          m_noiseAmplitude;
	float          m_signalAmplitude;
	float          m_balanceErr;
	float          m_amplitudeErr;
	unsigned int   m_blockSize;
	bool           m_swapIQ;
	IDataCallback* m_callback;
	int            m_id;
	float*         m_buffer;
	float*         m_awgn;
	unsigned int   m_noiseSize;
	float          m_cosVal;
	float          m_sinVal;
	float          m_cosDelta;
	float          m_sinDelta;
	unsigned int   m_awgnN;
};

#endif
