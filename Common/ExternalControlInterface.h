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

#ifndef	ExternalControlInterface_H
#define	ExternalControlInterface_H

#include "Frequency.h"
#include "Common.h"

#include <wx/wx.h>

class IExternalControlInterface {
public:
	virtual bool setExtTransmit(bool on) = 0;
	virtual bool setExtFrequency(const CFrequency& freq) = 0;
	virtual bool setExtMode(UWSDRMODE mode) = 0;
};

#endif
