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

#include "ExternalDialog.h"

#if defined(__WXMAC__)
const int BORDER_SIZE = 5;
const int TEXT_WIDTH  = 300;
#else
const int BORDER_SIZE = 5;
const int TEXT_WIDTH  = 130;
#endif

BEGIN_EVENT_TABLE(CExternalDialog, wxDialog)
	EVT_BUTTON(wxID_OK, CExternalDialog::onOK)
END_EVENT_TABLE()


CExternalDialog::CExternalDialog(wxWindow* parent, const wxString& title, const wxString& name, EXTERNALADDRS addr, int id) :
wxDialog(parent, id, title),
m_namesChoice(NULL),
m_addrsChoice(NULL),
m_name(name),
m_addr(addr)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* panel = new wxPanel(this, -1);

	wxFlexGridSizer* panelSizer = new wxFlexGridSizer(2);

	wxStaticText* label1 = new wxStaticText(panel, -1, _("Name"));
	panelSizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_namesChoice = new wxChoice(panel, -1, wxDefaultPosition, wxSize(TEXT_WIDTH, -1));
	m_namesChoice->Append(wxT("<none>"));
	m_namesChoice->Append(wxT("SDR 1"));
	m_namesChoice->Append(wxT("SDR 2"));
	m_namesChoice->Append(wxT("SDR 3"));
	m_namesChoice->Append(wxT("SDR 4"));
	panelSizer->Add(m_namesChoice, 0, wxALL, BORDER_SIZE);
	bool ret = m_namesChoice->SetStringSelection(name);
	if (!ret)
		m_namesChoice->SetSelection(0);

	wxStaticText* label2 = new wxStaticText(panel, -1, _("Addresses"));
	panelSizer->Add(label2, 0, wxALL, BORDER_SIZE);

	m_addrsChoice = new wxChoice(panel, -1, wxDefaultPosition, wxSize(TEXT_WIDTH, -1));
	m_addrsChoice->Append(_("Local Host"));
	m_addrsChoice->Append(_("Local Network"));
	m_addrsChoice->Append(_("Any"));
	panelSizer->Add(m_addrsChoice, 0, wxALL, BORDER_SIZE);
	m_addrsChoice->SetSelection(int(addr));

	panel->SetSizer(panelSizer);

	mainSizer->Add(panel);

	mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL, BORDER_SIZE);

	SetSizer(mainSizer);

	mainSizer->SetSizeHints(this);
}

CExternalDialog::~CExternalDialog()
{
}

void CExternalDialog::onOK(wxCommandEvent& WXUNUSED(event))
{
	long n1 = m_namesChoice->GetSelection();
	if (n1 == wxNOT_FOUND) {
		::wxMessageBox(_("The Name is not allowed to be empty"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	long n2 = m_addrsChoice->GetSelection();
	if (n2 == wxNOT_FOUND) {
		::wxMessageBox(_("The Addresses is not allowed to be empty"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	if (n1 != 0)
		m_name = m_namesChoice->GetStringSelection();
	else
		m_name.Clear();

	m_addr = EXTERNALADDRS(n2);

	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		Show(false);
	}
}

wxString CExternalDialog::getName() const
{
	return m_name;
}

EXTERNALADDRS CExternalDialog::getAddr() const
{
	return m_addr;
}
