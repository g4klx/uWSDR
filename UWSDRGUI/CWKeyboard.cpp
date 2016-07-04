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

#include "CWKeyboard.h"
#include "UWSDRApp.h"

#include <wx/statline.h>

/*
 * This is the lookup table to convert characters to morse code.
 */
static struct {
	const wxChar  character;
	const wxChar* cwString;
} cwLookup[] = {
	{wxT('A'), wxT(".-")},
	{wxT('B'), wxT("-...")},
	{wxT('C'), wxT("-.-.")},
	{wxT('D'), wxT("-..")},
	{wxT('E'), wxT(".")},
	{wxT('F'), wxT("..-.")},
	{wxT('G'), wxT("--.")},
	{wxT('H'), wxT("....")},
	{wxT('I'), wxT("..")},
	{wxT('J'), wxT(".---")},
	{wxT('K'), wxT("-.-")},
	{wxT('L'), wxT(".-..")},
	{wxT('M'), wxT("--")},
	{wxT('N'), wxT("-.")},
	{wxT('O'), wxT("---")},
	{wxT('P'), wxT(".--.")},
	{wxT('Q'), wxT("--.-")},
	{wxT('R'), wxT(".-.")},
	{wxT('S'), wxT("...")},
	{wxT('T'), wxT("-")},
	{wxT('U'), wxT("..-")},
	{wxT('V'), wxT("...-")},
	{wxT('W'), wxT(".--")},
	{wxT('X'), wxT("-..-")},
	{wxT('Y'), wxT("-.--")},
	{wxT('Z'), wxT("--..")},

	{wxT('1'), wxT(".----")},
	{wxT('2'), wxT("..---")},
	{wxT('3'), wxT("...--")},
	{wxT('4'), wxT("....-")},
	{wxT('5'), wxT(".....")},
	{wxT('6'), wxT("-....")},
	{wxT('7'), wxT("--...")},
	{wxT('8'), wxT("---..")},
	{wxT('9'), wxT("----.")},
	{wxT('0'), wxT("-----")},

	{wxT('/'), wxT("-..-.")},
	{wxT('?'), wxT("..--..")},
	{wxT(','), wxT("--..--")},
	{wxT('='), wxT("-...-")},
	{wxT('*'), wxT("...-.-")},
	{wxT('.'), wxT(".-.-.")},

	{wxT(' '), wxT(" ")}
};

const int cwLookupLen = sizeof(cwLookup) / sizeof(cwLookup[0]);

enum {
	REAL_TIME = 18344,
	BUTTON_TRANSMIT,
	BUTTON_ABORT
};

BEGIN_EVENT_TABLE(CCWKeyboard, wxDialog)
	EVT_TEXT(REAL_TIME,         CCWKeyboard::onRealTime)
	EVT_BUTTON(BUTTON_TRANSMIT, CCWKeyboard::onTransmit)
	EVT_BUTTON(BUTTON_ABORT,    CCWKeyboard::onAbort)
	EVT_BUTTON(wxID_HELP,       CCWKeyboard::onHelp)
END_EVENT_TABLE()

CCWKeyboard::CCWKeyboard(wxWindow* parent, int id) :
wxDialog(parent, id, wxString(_("CW Keyboard"))),
m_local(NULL),
m_remote(NULL),
m_locator(NULL),
m_report(NULL),
m_serial(NULL),
m_text(NULL),
m_button(NULL),
m_realTime(NULL),
m_speed(NULL),
m_abortButton(NULL),
m_prevLen(0U)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* entrySizer = new wxFlexGridSizer(6);

	wxStaticText* labelLocal = new wxStaticText(this, -1, _("My callsign (%M):"));
	entrySizer->Add(labelLocal, 0, wxALL, BORDER_SIZE);

	m_local = new wxTextCtrl(this, -1);
	m_local->SetMaxLength(12);
	entrySizer->Add(m_local, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelRemote = new wxStaticText(this, -1, _("Remote callsign (%R):"));
	entrySizer->Add(labelRemote, 0, wxALL, BORDER_SIZE);

	m_remote = new wxTextCtrl(this, -1);
	m_remote->SetMaxLength(12);
	entrySizer->Add(m_remote, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelLocator = new wxStaticText(this, -1, _("Locator (%Q):"));
	entrySizer->Add(labelLocator, 0, wxALL, BORDER_SIZE);

	m_locator = new wxTextCtrl(this, -1);
	m_locator->SetMaxLength(12);
	entrySizer->Add(m_locator, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelReport = new wxStaticText(this, -1, _("Report (%S):"));
	entrySizer->Add(labelReport, 0, wxALL, BORDER_SIZE);

	m_report = new wxTextCtrl(this, -1);
	m_report->SetMaxLength(8);
	entrySizer->Add(m_report, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelSerial = new wxStaticText(this, -1, _("Serial number (%N):"));
	entrySizer->Add(labelSerial, 0, wxALL, BORDER_SIZE);

	m_serial = new wxTextCtrl(this, -1);
	m_serial->SetMaxLength(8);
	entrySizer->Add(m_serial, 0, wxALL, BORDER_SIZE);

	wxStaticText* speedLabel = new wxStaticText(this, -1, _("Speed (WPM):"));
	entrySizer->Add(speedLabel, 0, wxALL, BORDER_SIZE);

	m_speed = new wxSpinCtrl(this, -1);
	m_speed->SetRange(5, 30);
	entrySizer->Add(m_speed, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(entrySizer, 0, wxALL, BORDER_SIZE);

	wxStaticLine* line1 = new wxStaticLine(this, -1, wxDefaultPosition, wxSize(CWKEYB_WIDTH, -1), wxLI_HORIZONTAL);
	mainSizer->Add(line1, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* textSizer = new wxFlexGridSizer(3);

	m_text   = new wxTextCtrl*[CWKEYBOARD_COUNT];
	m_button = new wxRadioButton*[CWKEYBOARD_COUNT + 1U];

	for (unsigned int i = 0U; i < CWKEYBOARD_COUNT; i++) {
		wxString text;
		text.Printf(_("Message %d:"), i + 1);

		wxStaticText* label = new wxStaticText(this, -1, text);
		textSizer->Add(label, 0, wxALL, BORDER_SIZE);

		m_text[i] = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(CWTEXT_WIDTH, -1));
		m_text[i]->SetMaxLength(200);
		textSizer->Add(m_text[i], 0, wxALL, BORDER_SIZE);

		m_button[i] = new wxRadioButton(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, (i == 0) ? wxRB_GROUP : 0);
		m_button[i]->SetValue(i == 0);
		textSizer->Add(m_button[i], 0, wxALL, BORDER_SIZE);
	}

	wxStaticText* labelRealTime = new wxStaticText(this, -1, _("Real Time:"));
	textSizer->Add(labelRealTime, 0, wxALL, BORDER_SIZE);

	m_realTime = new wxTextCtrl(this, REAL_TIME, wxEmptyString, wxDefaultPosition, wxSize(CWTEXT_WIDTH, REAL_TIME_HEIGHT));
	m_realTime->SetMaxLength(1000);
	textSizer->Add(m_realTime, 0, wxALL, BORDER_SIZE);

	m_button[CWKEYBOARD_COUNT] = new wxRadioButton(this, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	m_button[CWKEYBOARD_COUNT]->SetValue(false);
	textSizer->Add(m_button[CWKEYBOARD_COUNT], 0, wxALL, BORDER_SIZE);

	mainSizer->Add(textSizer, 0, wxALL, BORDER_SIZE);

	wxStaticLine* line2 = new wxStaticLine(this, -1, wxDefaultPosition, wxSize(CWKEYB_WIDTH, -1), wxLI_HORIZONTAL);
	mainSizer->Add(line2, 0, wxALL, BORDER_SIZE);

	wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* transmitButton = new wxButton(this, BUTTON_TRANSMIT, _("Transmit"));
	buttonSizer->Add(transmitButton, 0, wxALL, BORDER_SIZE);

	m_abortButton = new wxButton(this, BUTTON_ABORT, _("Abort"));
	buttonSizer->Add(m_abortButton, 0, wxALL, BORDER_SIZE);

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

CCWKeyboard::~CCWKeyboard()
{
}

void CCWKeyboard::setLocal(const wxString& text)
{
	m_local->SetValue(text);
}

wxString CCWKeyboard::getLocal() const
{
	return m_local->GetValue();
}

void CCWKeyboard::setRemote(const wxString& text)
{
	m_remote->SetValue(text);
}

wxString CCWKeyboard::getRemote() const
{
	return m_remote->GetValue();
}

void CCWKeyboard::setLocator(const wxString& text)
{
	m_locator->SetValue(text);
}

wxString CCWKeyboard::getLocator() const
{
	return m_locator->GetValue();
}

void CCWKeyboard::setReport(const wxString& text)
{
	m_report->SetValue(text);
}

wxString CCWKeyboard::getReport() const
{
	return m_report->GetValue();
}

void CCWKeyboard::setSerial(const wxString& text)
{
	m_serial->SetValue(text);
}

wxString CCWKeyboard::getSerial() const
{
	return m_serial->GetValue();
}

void CCWKeyboard::setMessage(unsigned int n, const wxString& text)
{
	wxASSERT(n < CWKEYBOARD_COUNT);

	m_text[n]->SetValue(text);
}

wxString CCWKeyboard::getMessage(unsigned int n) const
{
	wxASSERT(n < CWKEYBOARD_COUNT);

	return m_text[n]->GetValue();
}

void CCWKeyboard::setSpeed(unsigned int speed)
{
	wxASSERT(speed >= 5 && speed <= 30);

	m_speed->SetValue(speed);
}

unsigned int CCWKeyboard::getSpeed() const
{
	return m_speed->GetValue();
}

void CCWKeyboard::onHelp(wxCommandEvent& WXUNUSED(event))
{
	::wxGetApp().showHelp(500);
}

/*
 * Convert the input string to the required dashes, dots, and spaces. This is where
 * substitutions are also done.
 */
void CCWKeyboard::onTransmit(wxCommandEvent& WXUNUSED(event))
{
	wxString text;

	bool found = false;
	for (unsigned int i = 0; i < CWKEYBOARD_COUNT; i++) {
		if (m_button[i]->GetValue()) {
			text = m_text[i]->GetValue();
			found = true;
			break;
		}
	}

	// It could be the real time part ....
	if (!found) {
		if (!m_button[CWKEYBOARD_COUNT]->GetValue())
			return;

		CWERROR ret = ::wxGetApp().sendCW(0U, wxEmptyString, CW_TX_ON);

		switch (ret) {
			case CW_ERROR_MODE:
				::wxMessageBox(_("SDR is in the wrong mode"), _("UWSDR Error"), wxICON_ERROR);
				break;

			case CW_ERROR_TX:
				::wxMessageBox(_("Already sending a message"), _("UWSDR Error"), wxICON_ERROR);
				break;

			default:
				m_abortButton->SetLabel(_("End"));
				break;
		}

		return;
	}

	// Just in case we switch back to one of the non real-time messages
	m_abortButton->SetLabel(_("Abort"));

	text.UpperCase();

	text.Replace(wxT("%M"), m_local->GetValue(), true);
	text.Replace(wxT("%R"), m_remote->GetValue(), true);
	text.Replace(wxT("%Q"), m_locator->GetValue(), true);
	text.Replace(wxT("%S"), m_report->GetValue(), true);
	text.Replace(wxT("%N"), m_serial->GetValue(), true);

	text.UpperCase();

	wxString cwData = wxT(" ");

	for (unsigned int n = 0; n < text.length(); n++) {
		wxChar c = text.GetChar(n);

		for (int m = 0; m < cwLookupLen; m++)	{
			if (cwLookup[m].character == c) {
				cwData.Append(cwLookup[m].cwString);
				cwData.Append(wxT(" "));
				break;
			}
		}
	}

	CWERROR ret = ::wxGetApp().sendCW(getSpeed(), cwData, CW_SEND_TEXT);

	switch (ret) {
		case CW_ERROR_MODE:
			::wxMessageBox(_("SDR is in the wrong mode"), _("UWSDR Error"), wxICON_ERROR);
			break;

		case CW_ERROR_TX:
			::wxMessageBox(_("Already sending a message"), _("UWSDR Error"), wxICON_ERROR);
			break;

		default:
			break;
	}
}

void CCWKeyboard::onRealTime(wxCommandEvent& WXUNUSED(event))
{
	if (!m_button[CWKEYBOARD_COUNT]->GetValue())
		return;

	unsigned int length = m_realTime->GetLastPosition();

	wxString text = m_realTime->GetRange(m_prevLen, length);

	m_prevLen = length;

	text.UpperCase();

	wxString cwData;

	for (unsigned int n = 0; n < text.length(); n++) {
		wxChar c = text.GetChar(n);

		for (int m = 0; m < cwLookupLen; m++)	{
			if (cwLookup[m].character == c) {
				cwData.Append(cwLookup[m].cwString);
				cwData.Append(wxT(" "));
				break;
			}
		}
	}

	CWERROR ret = ::wxGetApp().sendCW(getSpeed(), cwData, CW_SEND_CHAR);

	switch (ret) {
		case CW_ERROR_MODE:
			::wxMessageBox(_("SDR is in the wrong mode"), _("UWSDR Error"), wxICON_ERROR);
			break;

		case CW_ERROR_TX:
			::wxMessageBox(_("Already sending a message"), _("UWSDR Error"), wxICON_ERROR);
			break;

		default:
			break;
	}
}

/*
 * Tell the system to stop transmitting.
 */
void CCWKeyboard::onAbort(wxCommandEvent& WXUNUSED(event))
{
	::wxGetApp().sendCW(0U, wxEmptyString, CW_STOP);
}
