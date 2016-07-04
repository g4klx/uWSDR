/*
 *   Copyright (C) 2002,2003,2006,2008 by Jonathan Naylor G4KLX
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

#include "Log.h"

#include <wx/datetime.h>
#include <wx/textfile.h>

CLog::CLog(const wxString& fileName) :
wxLog(),
m_file(NULL),
m_eol()
{
	m_file = new wxFile(fileName, wxFile::write);

	if (!m_file->IsOpened())
		return;

	m_eol = wxTextFile::GetEOL();
}

CLog::~CLog()
{
	delete m_file;
}

void CLog::DoLog(wxLogLevel level, const wxChar *msg, time_t timestamp)
{
	wxASSERT(m_file->IsOpened());
	wxASSERT(msg != NULL);

	wxString letter;

	switch (level) {
		case wxLOG_FatalError: letter = wxT("F"); break;
		case wxLOG_Error:      letter = wxT("E"); break;
		case wxLOG_Warning:    letter = wxT("W"); break;
		case wxLOG_Info:       letter = wxT("I"); break;
		case wxLOG_Message:    letter = wxT("M"); break;
		case wxLOG_Status:     letter = wxT("M"); break;
		case wxLOG_Trace:      letter = wxT("T"); break;
		case wxLOG_Debug:      letter = wxT("D"); break;
		default:               letter = wxT("U"); break;
	}

	wxDateTime dateTime(timestamp);
	wxString timeText = dateTime.Format(wxT(": %Y-%m-%d %H:%M:%S: "));

	wxString message = letter + timeText + msg + m_eol;

	DoLogString(message.c_str(), timestamp);

	if (level == wxLOG_FatalError || level == wxLOG_Error)
		m_file->Flush();

	if (level == wxLOG_FatalError)
		::abort();
}

void CLog::DoLogString(const wxChar* msg, time_t WXUNUSED(timestamp))
{
	wxASSERT(m_file->IsOpened());
	wxASSERT(msg != NULL);

	m_file->Write(msg);
}
