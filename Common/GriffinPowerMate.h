/*
 *   Copyright (C) 2008,2013 by Jonathan Naylor G4KLX
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

#ifndef	GriffinPowerMate_H
#define	GriffinPowerMate_H

#include <wx/wx.h>

#if defined(__WINDOWS__)
#include "libusb.h"
#else
#include <libusb-1.0/libusb.h>
#endif

#include "DialInterface.h"

class CGriffinPowerMate : public IDialInterface, public wxThread  {
public:
	CGriffinPowerMate();

	virtual void setCallback(IDialCallback* callback, int id);

	virtual bool open();
	virtual void close();

	virtual void* Entry();

protected:
	virtual ~CGriffinPowerMate();

private:
	IDialCallback*        m_callback;
	int                   m_id;
	unsigned char*        m_buffer;
	unsigned int          m_len;
	bool                  m_button;
	unsigned int          m_speed;
	bool                  m_killed;
	libusb_context*       m_context;
	libusb_device_handle* m_handle;
};

#endif
