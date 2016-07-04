/*
 *   Copyright (C) 2006-2007,2013 by Jonathan Naylor G4KLX
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

#include "NullWriter.h"

CNullWriter::CNullWriter()
{
}

CNullWriter::~CNullWriter()
{
}

bool CNullWriter::open(float WXUNUSED(sampleRate), unsigned int WXUNUSED(blockSize))
{
	wxLogMessage(wxT("NullWriter: started"));

	return true;
}

void CNullWriter::write(const float* buffer, unsigned int nSamples)
{
	wxASSERT(buffer != NULL);
	wxASSERT(nSamples > 0);
}

void CNullWriter::close()
{
	delete this;
}

void CNullWriter::enable(bool WXUNUSED(enable))
{
}

void CNullWriter::disable()
{
}
