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

#include "NullReader.h"

#include <wx/debug.h>
#include <wx/log.h>


CNullReader::CNullReader(IDataReader* reader) :
CThreadReader(reader),
m_blockSize(0),
m_buffer(NULL),
m_callback(NULL),
m_id(0)
{
}

CNullReader::~CNullReader()
{
	delete[] m_buffer;
}

void CNullReader::setCallback(IDataCallback* callback, int id)
{
	m_callback = callback;
	m_id       = id;
}

bool CNullReader::open(float sampleRate, unsigned int blockSize)
{
	m_blockSize = blockSize;

	m_buffer = new float[m_blockSize * 2];

	for (unsigned int i = 0; i < m_blockSize * 2; i++)
		m_buffer[i] = 0.0F;

	wxLogMessage(wxT("NullReader: started"));

	return CThreadReader::open(sampleRate, blockSize);
}

bool  CNullReader::create()
{
	wxASSERT(m_callback != NULL);

	m_callback->callback(m_buffer, m_blockSize, m_id);

	return true;
}
