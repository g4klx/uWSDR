/*
 *   Copyright (C) 2006,2013 by Jonathan Naylor G4KLX
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

#include "VoiceKeyboard.h"
#include "UWSDRApp.h"

#include <wx/statline.h>
#include <wx/file.h>


enum {
	BUTTON_BROWSE  = 25761,
	BUTTON_ABORT,
	BUTTON_TRANSMIT
};

BEGIN_EVENT_TABLE(CVoiceKeyboard, wxDialog)
	EVT_BUTTON(BUTTON_BROWSE,   CVoiceKeyboard::onBrowse)
	EVT_BUTTON(BUTTON_ABORT,    CVoiceKeyboard::onAbort)
	EVT_BUTTON(BUTTON_TRANSMIT, CVoiceKeyboard::onTransmit)
	EVT_BUTTON(wxID_HELP,       CVoiceKeyboard::onHelp)
END_EVENT_TABLE()

CVoiceKeyboard::CVoiceKeyboard(wxWindow* parent, int id) :
wxDialog(parent, id, wxString(_("Voice Keyer"))),
m_filename(NULL),
m_single(NULL),
m_continuous(NULL),
m_dir()
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* panelSizer = new wxFlexGridSizer(3);

	wxStaticText* label1 = new wxStaticText(this, -1, _("File Name:"));
	panelSizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_filename = new wxComboBox(this, -1, wxEmptyString, wxDefaultPosition, wxSize(VOICETEXT_WIDTH, -1));
	panelSizer->Add(m_filename, 0, wxALL, BORDER_SIZE);

	wxButton* browse = new wxButton(this, BUTTON_BROWSE, _("Browse..."));
	panelSizer->Add(browse, 0, wxALL, BORDER_SIZE);

	wxStaticText* label3 = new wxStaticText(this, -1, _("Repetition:"));
	panelSizer->Add(label3, 0, wxALL, BORDER_SIZE);

	m_single = new wxRadioButton(this, -1, _("Single"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	panelSizer->Add(m_single, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy2 = new wxStaticText(this, -1, wxEmptyString);
	panelSizer->Add(dummy2, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy3 = new wxStaticText(this, -1, wxEmptyString);
	panelSizer->Add(dummy3, 0, wxALL, BORDER_SIZE);

	m_continuous = new wxRadioButton(this, -1, _("Continuous"));
	panelSizer->Add(m_continuous, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(panelSizer);

	wxStaticLine* line2 = new wxStaticLine(this, -1, wxDefaultPosition, wxSize(VOICEKEYB_WIDTH, -1), wxLI_HORIZONTAL);
	mainSizer->Add(line2, 0, wxALL, BORDER_SIZE);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* transmitButton = new wxButton(this, BUTTON_TRANSMIT, _("Transmit"));
	buttonSizer->Add(transmitButton, 0, wxALL, BORDER_SIZE);

	wxButton* abortButton = new wxButton(this, BUTTON_ABORT, _("Abort"));
	buttonSizer->Add(abortButton, 0, wxALL, BORDER_SIZE);

	wxButton* closeButton = new wxButton(this, wxID_CANCEL, _("Close"));
	buttonSizer->Add(closeButton, 0, wxALL, BORDER_SIZE);

	wxButton* helpButton = new wxButton(this, wxID_HELP, _("Help"));
	buttonSizer->Add(helpButton, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(buttonSizer, 0, wxALL, BORDER_SIZE);

	SetAutoLayout(true);
	Layout();

	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);

	SetSizer(mainSizer);
}

CVoiceKeyboard::~CVoiceKeyboard()
{
}

void CVoiceKeyboard::setDir(const wxString& dir)
{
	m_dir = dir;
}

wxString CVoiceKeyboard::getDir() const
{
	return m_dir;
}

void CVoiceKeyboard::setFile(unsigned int n, const wxString& fileName)
{
	wxASSERT(n <= VOICEKEYER_COUNT);

	if (!fileName.IsEmpty())
		m_filename->Append(fileName);
}

wxString CVoiceKeyboard::getFile(unsigned int n) const
{
	wxASSERT(n <= VOICEKEYER_COUNT);

	return m_filename->GetString(n);
}

void CVoiceKeyboard::onBrowse(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog files(this, _("Choose a Wave File"), m_dir, wxEmptyString, _("WAV files (*.WAV)|*.wav;*.WAV"), wxFD_FILE_MUST_EXIST);
	int ret = files.ShowModal();

	if (ret != wxID_OK)
		return;

	m_dir = files.GetDirectory();

	wxString fileName = files.GetPath();

	m_filename->SetValue(fileName);
}

void CVoiceKeyboard::onHelp(wxCommandEvent& WXUNUSED(event))
{
	::wxGetApp().showHelp(103);
}

void CVoiceKeyboard::onTransmit(wxCommandEvent& WXUNUSED(event))
{
	wxString fileName = m_filename->GetValue();
	if (fileName.IsEmpty()) {
		::wxMessageBox(_("No file name entered or selected."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	int found = m_filename->FindString(fileName);
	if (found == wxNOT_FOUND)
		found = m_filename->Append(fileName);

	m_filename->SetSelection(found);

	bool ret = wxFile::Exists(fileName);
	if (!ret) {
		::wxMessageBox(_("Cannot find the file specified."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	VOICEERROR vret = VOICE_ERROR_NONE;

	if (m_single->GetValue())
		vret = ::wxGetApp().sendAudio(fileName, VOICE_SINGLE);

	if (m_continuous->GetValue())
		vret = ::wxGetApp().sendAudio(fileName, VOICE_CONTINUOUS);

	switch (vret) {
		case VOICE_ERROR_MODE:
			::wxMessageBox(_("SDR is in the wrong mode"), _("UWSDR Error"), wxICON_ERROR);
			break;

		case VOICE_ERROR_TX:
			::wxMessageBox(_("Already sending a file"), _("UWSDR Error"), wxICON_ERROR);
			break;

		case VOICE_ERROR_FILE:
			::wxMessageBox(_("Unable to open the sound file"), _("UWSDR Error"), wxICON_ERROR);
			break;

		default:
			break;
	}
}

void CVoiceKeyboard::onAbort(wxCommandEvent& WXUNUSED(event))
{
	::wxGetApp().sendAudio(wxEmptyString, VOICE_ABORT);
}
