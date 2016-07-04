/*
 *   Copyright (C) 2006-2007 by Jonathan Naylor G4KLX
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

#ifndef	ThreeToneReader_H
#define	ThreeToneReader_H

#include <wx/wx.h>

#include "ThreadReader.h"


class CThreeToneReader : public CThreadReader {

    public:
    CThreeToneReader(float frequency1, float frequency2, float frequency3, float amplitude, IDataReader* reader = NULL);

	virtual void setCallback(IDataCallback* callback, int id);

	virtual bool open(float sampleRate, unsigned int blockSize);

	virtual bool create();

    protected:
	virtual ~CThreeToneReader();

    private:
	float          m_frequency1;
	float          m_frequency2;
	float          m_frequency3;
	float          m_amplitude;
	unsigned int   m_blockSize;
	IDataCallback* m_callback;
	int            m_id;
	float*         m_buffer;
	float          m_cosVal1;
	float          m_sinVal1;
	float          m_cosDelta1;
	float          m_sinDelta1;
	float          m_cosVal2;
	float          m_sinVal2;
	float          m_cosDelta2;
	float          m_sinDelta2;
	float          m_cosVal3;
	float          m_sinVal3;
	float          m_cosDelta3;
	float          m_sinDelta3;
};

#endif
