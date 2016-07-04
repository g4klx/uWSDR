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

#include "ThreadReader.h"

#include <wx/debug.h>
#include <wx/log.h>


CThreadReader::CThreadReader(IDataReader* reader) :
wxThread(),
m_reader(reader),
m_run(),
m_exit(false)
{
}

CThreadReader::~CThreadReader()
{
	if (m_reader != NULL)
		m_reader->close();
}

bool CThreadReader::open(float sampleRate, unsigned int blockSize)
{
	if (m_reader != NULL) {
		m_reader->setCallback(this, 0);

		bool ret = m_reader->open(sampleRate, blockSize);
		if (!ret)
			return false;
	}

	Create();
	Run();

	return true;
}

void* CThreadReader::Entry()
{
	m_run.Wait();

	while (!m_exit) {
		bool ret = create();
		if (!ret)
			break;

		m_run.Wait();
	}

	return (void*)0;
}

void CThreadReader::close()
{
	m_exit = true;

	m_run.Post();
}

void CThreadReader::purge()
{
	wxSemaError status = m_run.TryWait();

	while (status != wxSEMA_BUSY) {
		m_run.Wait();

		status = m_run.TryWait();
	}
}

bool CThreadReader::hasClock()
{
	return m_reader != NULL;
}

void CThreadReader::clock()
{
	m_run.Post();
}

void CThreadReader::callback(float* buffer, unsigned int nSamples, int WXUNUSED(id))
{
	::memset(buffer, 0x00, nSamples * 2 * sizeof(float));

	clock();
}
