/*
 *   Copyright (C) 2013 by Jonathan Naylor G4KLX
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

#ifndef	ExternalProtocolHandler_H
#define	ExternalProtocolHandler_H

#include "ExternalControlInterface.h"
#include "UDPReaderWriter.h"
#include "DataCallback.h"
#include "Upsampler.h"
#include "Common.h"

#include <wx/wx.h>

enum CONNECTTYPE {
	CONNECTTYPE_NONE,
	CONNECTTYPE_CONTROL,
	CONNECTTYPE_AUDIO,
	CONNECTTYPE_RAW
};

class CExternalProtocolHandler {
public:
	CExternalProtocolHandler(float sampleRate, unsigned int blockSize, const wxString& name, EXTERNALADDRS addrs, IExternalControlInterface* iface);
	~CExternalProtocolHandler();

	bool open();

	void setCallback(IDataCallback* callback, int id);

	void clock();

	void writeRaw(const float* audio, unsigned int length);
	void writeAudio(const float* data, unsigned int length);

	void close();

private:
	float                      m_sampleRate;
	unsigned int               m_blockSize;
	unsigned int               m_skipFactor;
	CUpsampler*                m_upsampler;
	IExternalControlInterface* m_control;
	CUDPReaderWriter*          m_socket;
	EXTERNALADDRS              m_addrs;
	IDataCallback*             m_callback;
	int                        m_id;
	CONNECTTYPE                m_connected;
	bool                       m_transmit;
	unsigned char*             m_netBuffer;
	float*                     m_audioBuffer;
	in_addr                    m_remoteAddress;
	unsigned int               m_remotePort;
	unsigned int               m_pingInTimeout;
	unsigned int               m_pingOutTimeout;
	unsigned int               m_pingInTimer;
	unsigned int               m_pingOutTimer;
	unsigned char              m_outSerial;
	unsigned char              m_inSerial;
};

#endif
