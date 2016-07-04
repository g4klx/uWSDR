/*
 *   Copyright (C) 2008 by Jonathan Naylor G4KLX
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

#include "TuningDialog.h"

const int BORDER_SIZE    = 5;
const int HARDWARE_WIDTH = 200;

BEGIN_EVENT_TABLE(CTuningDialog, wxDialog)
	EVT_BUTTON(wxID_OK, CTuningDialog::onOK)
END_EVENT_TABLE()


CTuningDialog::CTuningDialog(wxWindow* parent, const wxString& title, TUNINGHW hw, int id) :
wxDialog(parent, id, title),
m_hwChoice(NULL),
m_hw(hw)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* panel = new wxPanel(this, -1);

	wxFlexGridSizer* panelSizer = new wxFlexGridSizer(1);

	m_hwChoice = new wxChoice(panel, -1, wxDefaultPosition, wxSize(HARDWARE_WIDTH, -1));
	m_hwChoice->Append(_("None"));
	m_hwChoice->Append(_("Griffin PowerMate"));
	panelSizer->Add(m_hwChoice, 0, wxALL, BORDER_SIZE);
	m_hwChoice->SetSelection(int(m_hw));

	panel->SetSizer(panelSizer);

	mainSizer->Add(panel);

	mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL, BORDER_SIZE);

	SetSizer(mainSizer);

	mainSizer->SetSizeHints(this);
}

CTuningDialog::~CTuningDialog()
{
}

void CTuningDialog::onOK(wxCommandEvent& WXUNUSED(event))
{
	long hw = m_hwChoice->GetSelection();
	if (hw == wxNOT_FOUND) {
		::wxMessageBox(_("The Hardware choice is not allowed to be empty"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	m_hw = TUNINGHW(hw);

	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		Show(false);
	}
}

TUNINGHW CTuningDialog::getTuningHW() const
{
	return m_hw;
}
