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

#ifndef	CWKeyer_H
#define	CWKeyer_H

#include <wx/wx.h>

#include "UWSDRDefs.h"
#include "ThreadReader.h"

class CCWKeyer : public CThreadReader {

    public:
    CCWKeyer();

	virtual bool open(float sampleRate, unsigned int blockSize);

	virtual bool create();

	virtual bool isActive() const;

	virtual void setCallback(IDataCallback* callback, int id);

	virtual CWERROR send(unsigned int speed, const wxString& text, CWSTATUS state);

	virtual void key(bool keyDown);

    protected:
	virtual ~CCWKeyer();

    private:
	float          m_sampleRate;
	unsigned int   m_blockSize;
	IDataCallback* m_callback;
	int            m_id;
	CWSTATUS       m_state;
	unsigned int   m_speed;
	wxString       m_text;
	bool*          m_bits;
	unsigned int   m_bitsLen;
	unsigned int   m_bitsIndex;
	bool           m_key;
	bool           m_lastKey;
	float*         m_buffer;
	unsigned int   m_defLen;
	float          m_cosDelta;
	float          m_sinDelta;
	float          m_cosValue;
	float          m_sinValue;

	unsigned int speedToUnits(unsigned int speed);
	void processKey(bool key, float* buffer, unsigned int blockSize);
	void createCW(const wxString& text, unsigned int speed);
	void end();
};

#endif
