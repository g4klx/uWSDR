/*
 *   Copyright (C) 2007,2013 by Jonathan Naylor G4KLX
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

#include "ConfigChooser.h"

#include "UWSDRDefs.h"

#include <wx/config.h>

const int DATA_WIDTH = 200;

BEGIN_EVENT_TABLE(CConfigChooser, wxDialog)
	EVT_BUTTON(wxID_OK,     CConfigChooser::onOK)
	EVT_BUTTON(wxID_CANCEL, CConfigChooser::onCancel)
END_EVENT_TABLE()


CConfigChooser::CConfigChooser(wxWindow* parent, int id) :
wxDialog(parent, id, wxString(_("UWSDR Config Chooser"))),
m_configChoice(NULL),
m_name()
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* panel = new wxPanel(this, -1);

	wxFlexGridSizer* panelSizer = new wxFlexGridSizer(2);

	wxStaticText* label = new wxStaticText(panel, -1, _("Name:"));
	panelSizer->Add(label, 0, wxALL, BORDER_SIZE);

	m_configChoice = new wxChoice(panel, -1, wxDefaultPosition, wxSize(DATA_WIDTH, -1));
	panelSizer->Add(m_configChoice, 0, wxALL, BORDER_SIZE);

	panel->SetSizer(panelSizer);

	mainSizer->Add(panel);

	mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL, BORDER_SIZE);

	SetSizer(mainSizer);

	mainSizer->SetSizeHints(this);

	enumerateConfigs();
}

CConfigChooser::~CConfigChooser()
{
}

void CConfigChooser::onOK(wxCommandEvent& WXUNUSED(event))
{
	int choice = m_configChoice->GetSelection();
	if (choice == wxNOT_FOUND) {
		::wxMessageBox(_("No SDR Name has been chosen"), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	m_name = m_configChoice->GetString(choice);

	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		Show(false);
	}
}

void CConfigChooser::onCancel(wxCommandEvent& WXUNUSED(event))
{
	if (IsModal()) {
		EndModal(wxID_CANCEL);
	} else {
		SetReturnCode(wxID_CANCEL);
		Show(false);
	}
}

void CConfigChooser::enumerateConfigs()
{
	wxConfig* config = new wxConfig(APPNAME);
	wxASSERT(config != NULL);

	wxString name;
	long index;

	bool cont = config->GetFirstGroup(name, index);
	while (cont) {
		m_configChoice->Append(name);

		cont = config->GetNextGroup(name, index);
	}

	if (m_configChoice->GetCount() > 0U)
		m_configChoice->SetSelection(0);

	delete config;
}

wxString CConfigChooser::getName() const
{
	return m_name;
}
