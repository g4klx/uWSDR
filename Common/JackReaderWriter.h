/*
 *   Copyright (C) 2007 by Jonathan Naylor G4KLX
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

#ifndef	JackReaderWriter_H
#define	JackReaderWriter_H

#include <wx/wx.h>

#include "DataReader.h"
#include "DataWriter.h"
#include "DataCallback.h"
#include "RingBuffer.h"

#if defined(USE_JACK)

#include <jack/jack.h>


extern "C" {
	int jcwCallback(jack_nframes_t nFrames, void* userData);
}


class CJackReaderWriter : public IDataWriter, public IDataReader {
    public:
	CJackReaderWriter(const wxString& name, unsigned int inChannels, unsigned int outChannels);

	virtual void setCallback(IDataCallback* callback, int id);

	virtual bool open(float sampleRate, unsigned int blockSize);
	virtual void write(const float* buffer, unsigned int nSamples);
	virtual void close();

	virtual void enable(bool enable = true);
	virtual void disable();

	virtual void purge();

	virtual int  callback(jack_nframes_t nFrames);

	virtual bool hasClock();
	virtual void clock();

    protected:
	virtual ~CJackReaderWriter();

    private:
	wxString       m_name;
	unsigned int   m_inChannels;
	unsigned int   m_outChannels;
	unsigned int   m_blockSize;
	IDataCallback* m_callback;
	int            m_id;
	jack_client_t* m_client;
	jack_port_t*   m_inPortI;
	jack_port_t*   m_inPortQ;
	jack_port_t*   m_outPortI;
	jack_port_t*   m_outPortQ;
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
};

#else

class CJackReaderWriter : public IDataWriter, public IDataReader {
    public:
	CJackReaderWriter(const wxString& name, unsigned int inChannels, unsigned int outChannels);

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
	virtual ~CJackReaderWriter();

    private:
};

#endif

#endif
