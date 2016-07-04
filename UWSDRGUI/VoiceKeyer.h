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

#ifndef	VoiceKeyer_H
#define	VoiceKeyer_H

#include <wx/wx.h>

#include "UWSDRDefs.h"
#include "DataReader.h"
#include "DataCallback.h"
#include "SoundFileReader.h"


class CVoiceKeyer : public IDataReader, public IDataCallback {

    public:
    CVoiceKeyer();

	virtual void callback(float* buffer, unsigned int nSamples, int id);

	virtual bool open(float sampleRate, unsigned int blockSize);
	virtual void close();

	virtual void purge();

	virtual bool hasClock();
	virtual void clock();

	virtual bool isActive() const;

	virtual void setCallback(IDataCallback* callback, int id);

	virtual VOICEERROR send(const wxString& fileName, VOICESTATUS state);

    protected:
	virtual ~CVoiceKeyer();

    private:
	float             m_sampleRate;
	unsigned int      m_blockSize;
	IDataCallback*    m_callback;
	int               m_id;
	VOICESTATUS       m_status;
	CSoundFileReader* m_file;
};

#endif
