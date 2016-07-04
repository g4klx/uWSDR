/*
 *   Copyright (C) 2006,2008 by Jonathan Naylor G4KLX
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

#include "FreqKeypad.h"
#include "UWSDRDefs.h"

enum {
	BUTTON_1 = 15342,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
	BUTTON_5,
	BUTTON_6,
	BUTTON_7,
	BUTTON_8,
	BUTTON_9,
	BUTTON_0,
	BUTTON_POINT,
	BUTTON_C
};

BEGIN_EVENT_TABLE(CFreqKeypad, wxDialog)
	EVT_BUTTON(BUTTON_1,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_2,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_3,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_4,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_5,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_6,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_7,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_8,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_9,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_0,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_C,     CFreqKeypad::onButton)
	EVT_BUTTON(BUTTON_POINT, CFreqKeypad::onButton)
	EVT_BUTTON(wxID_OK,      CFreqKeypad::onOK)
END_EVENT_TABLE()

CFreqKeypad::CFreqKeypad(wxWindow* parent, int id, const CFrequency& vfo, const CFrequency& minFreq, const CFrequency& maxFreq) :
wxDialog(parent, id, wxString(_("Frequency Keypad"))),
m_vfo(vfo),
m_minFreq(minFreq),
m_maxFreq(maxFreq),
m_frequency(),
m_text(NULL)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	m_text = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, wxSize(FREQPAD_WIDTH * 3, -1));
	m_text->SetMaxLength(12);
	mainSizer->Add(m_text, 0, wxALL, BORDER_SIZE);

	wxGridSizer* buttonSizer = new wxGridSizer(3);

	wxButton* button1 = new wxButton(this, BUTTON_1, wxT("1"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button1);

	wxButton* button2 = new wxButton(this, BUTTON_2, wxT("2"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button2);

	wxButton* button3 = new wxButton(this, BUTTON_3, wxT("3"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button3);

	wxButton* button4 = new wxButton(this, BUTTON_4, wxT("4"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button4);

	wxButton* button5 = new wxButton(this, BUTTON_5, wxT("5"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button5);

	wxButton* button6 = new wxButton(this, BUTTON_6, wxT("6"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button6);

	wxButton* button7 = new wxButton(this, BUTTON_7, wxT("7"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button7);

	wxButton* button8 = new wxButton(this, BUTTON_8, wxT("8"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button8);

	wxButton* button9 = new wxButton(this, BUTTON_9, wxT("9"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button9);

	wxButton* buttonPoint = new wxButton(this, BUTTON_POINT, wxT("."), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(buttonPoint);

	wxButton* button0 = new wxButton(this, BUTTON_0, wxT("0"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(button0);

	wxButton* buttonC = new wxButton(this, BUTTON_C, wxT("C"), wxDefaultPosition, wxSize(FREQPAD_WIDTH, FREQPAD_HEIGHT));
	buttonSizer->Add(buttonC);

	mainSizer->Add(buttonSizer, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(CreateButtonSizer(wxOK | wxCANCEL), 0, wxALL, BORDER_SIZE);

	SetAutoLayout(true);
	Layout();

	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);

	SetSizer(mainSizer);
}

CFreqKeypad::~CFreqKeypad()
{
}

CFrequency CFreqKeypad::getFrequency() const
{
	return m_frequency;
}

void CFreqKeypad::onButton(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case BUTTON_0:
			m_text->AppendText(wxT("0"));
			return;
		case BUTTON_1:
			m_text->AppendText(wxT("1"));
			return;
		case BUTTON_2:
			m_text->AppendText(wxT("2"));
			return;
		case BUTTON_3:
			m_text->AppendText(wxT("3"));
			return;
		case BUTTON_4:
			m_text->AppendText(wxT("4"));
			return;
		case BUTTON_5:
			m_text->AppendText(wxT("5"));
			return;
		case BUTTON_6:
			m_text->AppendText(wxT("6"));
			return;
		case BUTTON_7:
			m_text->AppendText(wxT("7"));
			return;
		case BUTTON_8:
			m_text->AppendText(wxT("8"));
			return;
		case BUTTON_9:
			m_text->AppendText(wxT("9"));
			return;
		case BUTTON_POINT:
			m_text->AppendText(wxT("."));
			return;
		case BUTTON_C: {
				int len = m_text->GetLastPosition();
				m_text->Remove(len - 1, len);
			}
			return;
	}
}

void CFreqKeypad::onOK(wxCommandEvent& WXUNUSED(event))
{
	wxString text = m_text->GetValue();

	if (text.length() == 0) {
		::wxBell();
		return;
	}

	if (text.GetChar(0) == wxT('.')) {
		wxString mhz = m_vfo.getString().BeforeFirst(wxT('.'));
		text.Prepend(mhz);
	}

	if (!m_frequency.set(text)) {
		::wxBell();
		return;
	}

	if (m_frequency >= m_maxFreq || m_frequency < m_minFreq) {
		::wxBell();
		return;
	}

	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		Show(false);
	}
}
