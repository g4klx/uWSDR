/*
 *   Copyright (C) 2006 by Jonathan Naylor G4KLX
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

#ifndef	DataReader_H
#define	DataReader_H

#include "DataCallback.h"

class IDataReader {
    public:
	virtual bool open(float sampleRate, unsigned int blockSize) = 0;
	virtual void setCallback(IDataCallback* callback, int id) = 0;
	virtual void close() = 0;

	virtual void purge() = 0;

	virtual bool hasClock() = 0;
	virtual void clock() = 0;
};

#endif
