/*
 *   Copyright (C) 2006-2008 by Jonathan Naylor G4KLX
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

#ifndef	SoundCardReaderWriter_H
#define	SoundCardReaderWriter_H

#include <wx/wx.h>

#include "DataReader.h"
#include "DataWriter.h"
#include "DataCallback.h"
#include "RingBuffer.h"

#if defined(USE_PORTAUDIO)

#include "portaudio.h"


extern "C" {
	int scwCallback(const void* input, void* output, unsigned long nSamples, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);
}


class CSoundCardReaderWriter : public IDataWriter, public IDataReader {
    public:
	CSoundCardReaderWriter(int inDev, int outDev, unsigned int inChannels, unsigned int outChannels);

	virtual void setCallback(IDataCallback* callback, int id);

	virtual bool open(float sampleRate, unsigned int blockSize);
	virtual void write(const float* buffer, unsigned int nSamples);
	virtual void close();

	virtual void enable(bool enable = true);
	virtual void disable();

	virtual void purge();

	virtual int  callback(const void* input, void* output, unsigned long nSamples, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags);

	virtual bool hasClock();
	virtual void clock();

    protected:
	virtual ~CSoundCardReaderWriter();

    private:
	int            m_inDev;
	int            m_outDev;
	unsigned int   m_inChannels;
	unsigned int   m_outChannels;
	unsigned int   m_blockSize;
	IDataCallback* m_callback;
	int            m_id;
	PaStream*      m_stream;
	CRingBuffer*   m_buffer;
	float*         m_lastBuffer;
	float*         m_inBuffer;
	float*         m_outBuffer;
	unsigned int   m_requests;
	unsigned int   m_underruns;
	unsigned int   m_overruns;
	bool           m_enabled;
	unsigned int   m_opened;
	bool           m_active;

	void writeLog(const wxString& function, PaError error) const;
};

#else

class CSoundCardReaderWriter : public IDataWriter, public IDataReader {
    public:
	CSoundCardReaderWriter(int inDev, int outDev, unsigned int inChannels, unsigned int outChannels);

	virtual void setCallback(IDataCallback* callback, int id);

	virtual bool open(float sampleRate, unsigned int blockSize);
	virtual void write(const float* buffer, unsigned int nSamples);
	virtual void close();

	virtual void enable(bool enable = true);
	virtual void disable();

	virtual void purge();

	virtual bool hasClock();
	virtual void clock();

    protected:
	virtual ~CSoundCardReaderWriter();

    private:
};

#endif

#endif
