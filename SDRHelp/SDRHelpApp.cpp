/*
 *   Copyright (C) 2006-2008,2013 by Jonathan Naylor G4KLX
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

#include "SDRHelpApp.h"

#include <wx/image.h>
#include <wx/fs_zip.h>



IMPLEMENT_APP(CSDRHelpApp)

CSDRHelpApp::CSDRHelpApp() :
wxApp(),
m_help(NULL)
{
}

CSDRHelpApp::~CSDRHelpApp()
{
}

bool CSDRHelpApp::OnInit()
{
	if (!wxApp::OnInit())
		return false;

	::wxInitAllImageHandlers();

	wxFileSystem::AddHandler(new wxZipFSHandler);

	m_help = new wxHtmlHelpController();
	m_help->SetTitleFormat(_("UWSDR Help: %s"));

	wxFileName fileName;
	fileName.AssignDir(getHelpDir());

	fileName.SetFullName(wxT("GUISetup.zip"));
	m_help->AddBook(fileName);

	// fileName.SetFullName(wxT("SDRSetup.zip"));
	// m_help->AddBook(fileName);

	fileName.SetFullName(wxT("UWSDR.zip"));
	m_help->AddBook(fileName);

	m_help->DisplayContents();

	return true;
}

int CSDRHelpApp::OnExit()
{
	delete m_help;

	return 0;
}

wxString CSDRHelpApp::getHelpDir()
{
#if defined(__WXMSW__)
	wxConfig* config = new wxConfig(wxT("UWSDR"));
	wxASSERT(config != NULL);

	wxString dir;
	bool ret = config->Read(wxT("/InstPath"), &dir);

	if (!ret) {
		delete config;
		return wxEmptyString;
	}

	delete config;

	return dir;
#else
	return wxT(DATA_DIR);
#endif
}
