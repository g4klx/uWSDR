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

#include "UWSDRPreferences.h"
#include "UWSDRDefs.h"
#include "UWSDRApp.h"

const int SLIDER_WIDTH = 225;

enum {
	RXIQ_PHASE = 8763,
	RXIQ_GAIN,
	TXIQ_PHASE,
	TXIQ_GAIN,
	RXRF_GAIN,
	TXEQ_ENABLE,
	TXEQ_PREAMP,
	TXEQ_GAIN0,
	TXEQ_GAIN1,
	TXEQ_GAIN2
};

BEGIN_EVENT_TABLE(CUWSDRPreferences, wxDialog)
	EVT_SPINCTRL(RXIQ_PHASE,        CUWSDRPreferences::onIQChanged)
	EVT_SPINCTRL(RXIQ_GAIN,         CUWSDRPreferences::onIQChanged)
	EVT_SPINCTRL(TXIQ_PHASE,        CUWSDRPreferences::onIQChanged)
	EVT_SPINCTRL(TXIQ_GAIN,         CUWSDRPreferences::onIQChanged)
	EVT_COMMAND_SCROLL(RXRF_GAIN,   CUWSDRPreferences::onRFGainChanged)
	EVT_CHOICE(TXEQ_ENABLE,         CUWSDRPreferences::onTXEQEnabled)
	EVT_COMMAND_SCROLL(TXEQ_PREAMP, CUWSDRPreferences::onTXEQChanged)
	EVT_COMMAND_SCROLL(TXEQ_GAIN0,  CUWSDRPreferences::onTXEQChanged)
	EVT_COMMAND_SCROLL(TXEQ_GAIN1,  CUWSDRPreferences::onTXEQChanged)
	EVT_COMMAND_SCROLL(TXEQ_GAIN2,  CUWSDRPreferences::onTXEQChanged)
	EVT_BUTTON(wxID_OK,             CUWSDRPreferences::onOK)
	EVT_BUTTON(wxID_HELP,           CUWSDRPreferences::onHelp)
END_EVENT_TABLE()

CUWSDRPreferences::CUWSDRPreferences(wxWindow* parent, int id, CSDRParameters* parameters, CDSPControl* dsp) :
wxDialog(parent, id, wxString(_("UWSDR Preferences"))),
m_parameters(parameters),
m_dsp(dsp),
m_noteBook(NULL),
m_freqShift1(NULL),
m_freqShift2(NULL),
m_freqOffset(NULL),
m_deviationFMW(NULL),
m_deviationFMN(NULL),
m_agcAM(NULL),
m_agcSSB(NULL),
m_agcCW(NULL),
m_agcDig(NULL),
m_filterFMW(NULL),
m_filterFMN(NULL),
m_filterAM(NULL),
m_filterSSB(NULL),
m_filterCWW(NULL),
m_filterCWN(NULL),
m_filterDig(NULL),
m_tuningFM(NULL),
m_tuningAM(NULL),
m_tuningSSB(NULL),
m_tuningCWW(NULL),
m_tuningCWN(NULL),
m_tuningDig(NULL),
m_stepVeryFast(NULL),
m_stepFast(NULL),
m_stepMedium(NULL),
m_stepSlow(NULL),
m_stepVerySlow(NULL),
m_nbButton(NULL),
m_nbValue(NULL),
m_nb2Button(NULL),
m_nb2Value(NULL),
m_spButton(NULL),
m_spValue(NULL),
m_binButton(NULL),
m_panValue(NULL),
m_recordType(NULL),
m_carrierLevel(NULL),
m_alcAttack(NULL),
m_alcDecay(NULL),
m_alcHang(NULL),
m_rfValue(NULL),
m_rxIQPhase(NULL),
m_rxIQGain(NULL),
m_txIQPhase(NULL),
m_txIQGain(NULL),
m_method(NULL),
m_txEqEnable(NULL),
m_txEqPreamp(NULL),
m_txEqGain0(NULL),
m_txEqGain1(NULL),
m_txEqGain2(NULL)
{
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	m_noteBook = new wxNotebook(this, -1);

	m_noteBook->AddPage(createFrequencyTab(m_noteBook), _("Frequencies"), true);

	m_noteBook->AddPage(createModeTab(m_noteBook), _("Modes"), false);

	m_noteBook->AddPage(createStepTab(m_noteBook), _("Step Size"), false);

	m_noteBook->AddPage(createReceiveTab(m_noteBook), _("Receive"), false);

	m_noteBook->AddPage(createTransmitTab(m_noteBook), _("Transmit"), false);

	m_noteBook->AddPage(createALCTab(m_noteBook), _("ALC"), false);

	m_noteBook->AddPage(createIQTab(m_noteBook), _("I + Q"), false);

	m_noteBook->AddPage(createTXEQTab(m_noteBook), _("TX EQ"), false);

	mainSizer->Add(m_noteBook, 1, wxALL | wxGROW, BORDER_SIZE);

	mainSizer->Add(CreateButtonSizer(wxHELP | wxOK | wxCANCEL), 0, wxALL | wxALIGN_RIGHT, BORDER_SIZE);

	SetAutoLayout(true);
	Layout();

	mainSizer->Fit(this);
	mainSizer->SetSizeHints(this);

	SetSizer(mainSizer);

	wxString text;
	text.Printf(wxT("%u"), m_parameters->m_freqShift1 / 1000);
	m_freqShift1->SetValue(text);

	text.Printf(wxT("%u"), m_parameters->m_freqShift2 / 1000);
	m_freqShift2->SetValue(text);

	text.Printf(wxT("%.1lf"), m_parameters->m_freqOffset);
	m_freqOffset->SetValue(text);

	m_agcAM->SetSelection(m_parameters->m_agcAM);
	m_agcSSB->SetSelection(m_parameters->m_agcSSB);
	m_agcCW->SetSelection(m_parameters->m_agcCW);
	m_agcDig->SetSelection(m_parameters->m_agcDig);

	m_deviationFMW->SetSelection(m_parameters->m_deviationFMW);
	m_deviationFMN->SetSelection(m_parameters->m_deviationFMN);

	m_filterFMW->SetSelection(m_parameters->m_filterFMW);
	m_filterFMN->SetSelection(m_parameters->m_filterFMN);
	m_filterAM->SetSelection(m_parameters->m_filterAM);
	m_filterSSB->SetSelection(m_parameters->m_filterSSB);
	m_filterCWW->SetSelection(m_parameters->m_filterCWW);
	m_filterCWN->SetSelection(m_parameters->m_filterCWN);
	m_filterDig->SetSelection(m_parameters->m_filterDig);

	m_tuningFM->SetSelection(m_parameters->m_vfoSpeedFM);
	m_tuningAM->SetSelection(m_parameters->m_vfoSpeedAM);
	m_tuningSSB->SetSelection(m_parameters->m_vfoSpeedSSB);
	m_tuningCWW->SetSelection(m_parameters->m_vfoSpeedCWW);
	m_tuningCWN->SetSelection(m_parameters->m_vfoSpeedCWN);
	m_tuningDig->SetSelection(m_parameters->m_vfoSpeedDig);

	text.Printf(wxT("%.1f"), m_parameters->m_stepVeryFast);
	m_stepVeryFast->SetValue(text);
	text.Printf(wxT("%.1f"), m_parameters->m_stepFast);
	m_stepFast->SetValue(text);
	text.Printf(wxT("%.1f"), m_parameters->m_stepMedium);
	m_stepMedium->SetValue(text);
	text.Printf(wxT("%.1f"), m_parameters->m_stepSlow);
	m_stepSlow->SetValue(text);
	text.Printf(wxT("%.1f"), m_parameters->m_stepVerySlow);
	m_stepVerySlow->SetValue(text);

	m_nbButton->SetValue(m_parameters->m_nbOn);
	m_nbValue->SetValue(m_parameters->m_nbValue);
	m_nb2Button->SetValue(m_parameters->m_nb2On);
	m_nb2Value->SetValue(m_parameters->m_nb2Value);

	switch (m_parameters->m_recordType) {
		case RECORD_MONO_AUDIO:
			m_recordType->SetSelection(0);
			break;
		case RECORD_STEREO_AUDIO:
			m_recordType->SetSelection(1);
			break;
		case RECORD_STEREO_IQ:
			m_recordType->SetSelection(2);
			break;
	}

	m_binButton->SetValue(m_parameters->m_binaural);
	m_panValue->SetValue(m_parameters->m_pan);

	// Map 500 -> 1000 to -30 -> 0
	unsigned int val = (unsigned int)(10.0 * ::log10(double(m_parameters->m_rfGain) / 1000.0) + 0.5);
	m_rfValue->SetValue(val);

	m_spButton->SetValue(m_parameters->m_spOn);
	m_spValue->SetValue(m_parameters->m_spValue);
	m_carrierLevel->SetValue(m_parameters->m_carrierLevel);

	m_alcAttack->SetValue(m_parameters->m_alcAttack);
	m_alcDecay->SetValue(m_parameters->m_alcDecay);
	m_alcHang->SetValue(m_parameters->m_alcHang);

	m_rxIQPhase->SetValue(m_parameters->m_rxIQphase);
	m_rxIQGain->SetValue(m_parameters->m_rxIQgain);

	m_txIQPhase->SetValue(m_parameters->m_txIQphase);
	m_txIQGain->SetValue(m_parameters->m_txIQgain);

	m_method->SetSelection(m_parameters->m_weaver ? 1 : 0);

	m_txEqEnable->SetSelection(m_parameters->m_txEqOn ? 1 : 0);
	m_txEqPreamp->SetValue(m_parameters->m_txEqPreamp);
	m_txEqGain0->SetValue(m_parameters->m_txEqGain0);
	m_txEqGain1->SetValue(m_parameters->m_txEqGain1);
	m_txEqGain2->SetValue(m_parameters->m_txEqGain2);

	if (m_parameters->m_hardwareStepSize >= 100.0F     ||
	    m_parameters->m_hardwareType == TYPE_SI570RX   ||
	    m_parameters->m_hardwareType == TYPE_SI570TXRX ||
	    m_parameters->m_hardwareType == TYPE_DEMO)
		m_method->Disable();

	if (m_parameters->m_hardwareTXRange.isEmpty()) {
		m_freqShift1->Disable();
		m_freqShift2->Disable();

		m_deviationFMW->Disable();
		m_deviationFMN->Disable();

		m_spButton->Disable();
		m_spValue->Disable();
		m_carrierLevel->Disable();

		m_alcAttack->Disable();
		m_alcDecay->Disable();
		m_alcHang->Disable();

		m_txIQPhase->Disable();
		m_txIQGain->Disable();
	}
}

CUWSDRPreferences::~CUWSDRPreferences()
{
}

void CUWSDRPreferences::onOK(wxCommandEvent& WXUNUSED(event))
{
	wxString text = m_freqShift1->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("Shift 1 may not be empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	long shift1;
	bool ret = text.ToLong(&shift1);

	if (!ret) {
		::wxMessageBox(_("Shift 1 is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (shift1 < 0L) {
		::wxMessageBox(_("Shift 1 may not be negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_freqShift2->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("Shift 2 may not be empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	long shift2;
	ret = text.ToLong(&shift2);

	if (!ret) {
		::wxMessageBox(_("Shift 2 is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (shift2 < 0L) {
		::wxMessageBox(_("Shift 2 may not be negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_freqOffset->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("The Offset may not empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	double offset;
	ret = text.ToDouble(&offset);

	if (!ret) {
		::wxMessageBox(_("The offset is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_stepVeryFast->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("The Very fast step size may not empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	double stepVeryFast;
	ret = text.ToDouble(&stepVeryFast);

	if (!ret) {
		::wxMessageBox(_("The Very fast step size is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (stepVeryFast <= 0.0) {
		::wxMessageBox(_("The Very fast step size may not be zero or negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_stepFast->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("The Fast step size may not empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	double stepFast;
	ret = text.ToDouble(&stepFast);

	if (!ret) {
		::wxMessageBox(_("The Fast step size is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (stepFast <= 0.0) {
		::wxMessageBox(_("The Fast step size may not be zero or negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_stepMedium->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("The Medium step size may not empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	double stepMedium;
	ret = text.ToDouble(&stepMedium);

	if (!ret) {
		::wxMessageBox(_("The Medium step size is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (stepMedium <= 0.0) {
		::wxMessageBox(_("The Medium step size may not be zero or negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_stepSlow->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("The Slow step size may not empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	double stepSlow;
	ret = text.ToDouble(&stepSlow);

	if (!ret) {
		::wxMessageBox(_("The Slow step size is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (stepSlow <= 0.0) {
		::wxMessageBox(_("The Slow step size may not be zero or negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	text = m_stepVerySlow->GetValue();

	if (text.IsEmpty()) {
		::wxMessageBox(_("The Very slow step size may not empty."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	double stepVerySlow;
	ret = text.ToDouble(&stepVerySlow);

	if (!ret) {
		::wxMessageBox(_("The Very slow step size is not a valid number."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	if (stepVerySlow <= 0.0) {
		::wxMessageBox(_("The Very slow step size may not be zero or negative."), _("UWSDR Error"), wxICON_ERROR);
		return;
	}

	m_parameters->m_freqShift1 = shift1 * 1000U;
	m_parameters->m_freqShift2 = shift2 * 1000U;
	m_parameters->m_freqOffset = offset;

	m_parameters->m_deviationFMW = FMDEVIATION(m_deviationFMW->GetSelection());
	m_parameters->m_deviationFMN = FMDEVIATION(m_deviationFMN->GetSelection());

	m_parameters->m_agcAM  = AGCSPEED(m_agcAM->GetSelection());
	m_parameters->m_agcSSB = AGCSPEED(m_agcSSB->GetSelection());
	m_parameters->m_agcCW  = AGCSPEED(m_agcCW->GetSelection());
	m_parameters->m_agcDig = AGCSPEED(m_agcDig->GetSelection());

	m_parameters->m_filterFMW = FILTERWIDTH(m_filterFMW->GetSelection());
	m_parameters->m_filterFMN = FILTERWIDTH(m_filterFMN->GetSelection());
	m_parameters->m_filterAM  = FILTERWIDTH(m_filterAM->GetSelection());
	m_parameters->m_filterSSB = FILTERWIDTH(m_filterSSB->GetSelection());
	m_parameters->m_filterCWW = FILTERWIDTH(m_filterCWW->GetSelection());
	m_parameters->m_filterCWN = FILTERWIDTH(m_filterCWN->GetSelection());
	m_parameters->m_filterDig = FILTERWIDTH(m_filterDig->GetSelection());

	m_parameters->m_vfoSpeedFM  = VFOSPEED(m_tuningFM->GetSelection());
	m_parameters->m_vfoSpeedAM  = VFOSPEED(m_tuningAM->GetSelection());
	m_parameters->m_vfoSpeedSSB = VFOSPEED(m_tuningSSB->GetSelection());
	m_parameters->m_vfoSpeedCWW = VFOSPEED(m_tuningCWW->GetSelection());
	m_parameters->m_vfoSpeedCWN = VFOSPEED(m_tuningCWN->GetSelection());
	m_parameters->m_vfoSpeedDig = VFOSPEED(m_tuningDig->GetSelection());

	m_parameters->m_stepVeryFast = stepVeryFast;
	m_parameters->m_stepFast     = stepFast;
	m_parameters->m_stepMedium   = stepMedium;
	m_parameters->m_stepSlow     = stepSlow;
	m_parameters->m_stepVerySlow = stepVerySlow;

	m_parameters->m_nbOn     = m_nbButton->IsChecked();
	m_parameters->m_nbValue  = m_nbValue->GetValue();
	m_parameters->m_nb2On    = m_nb2Button->IsChecked();
	m_parameters->m_nb2Value = m_nb2Value->GetValue();

	switch (m_recordType->GetSelection()) {
		case 0:
			m_parameters->m_recordType = RECORD_MONO_AUDIO;
			break;
		case 1:
			m_parameters->m_recordType = RECORD_STEREO_AUDIO;
			break;
		case 2:
			m_parameters->m_recordType = RECORD_STEREO_IQ;
			break;
	}

	m_parameters->m_binaural = m_binButton->GetValue();
	m_parameters->m_pan      = m_panValue->GetValue();

	// Map -30 -> 0 to 1 -> 1000
	double gainDb = double(m_rfValue->GetValue());
	m_parameters->m_rfGain    = (unsigned int)(1000.0 * ::pow(10.0, gainDb / 10.0) + 0.5);

	m_parameters->m_spOn         = m_spButton->IsChecked();
	m_parameters->m_spValue      = m_spValue->GetValue();
	m_parameters->m_carrierLevel = m_carrierLevel->GetValue();

	m_parameters->m_alcAttack = m_alcAttack->GetValue();
	m_parameters->m_alcDecay  = m_alcDecay->GetValue();
	m_parameters->m_alcHang   = m_alcHang->GetValue();

	m_parameters->m_txIQphase = m_txIQPhase->GetValue();
	m_parameters->m_txIQgain  = m_txIQGain->GetValue();

	m_parameters->m_rxIQphase = m_rxIQPhase->GetValue();
	m_parameters->m_rxIQgain  = m_rxIQGain->GetValue();

	m_parameters->m_weaver = m_method->GetSelection() == 1;

	m_parameters->m_txEqOn     = m_txEqEnable->GetSelection() == 1;
	m_parameters->m_txEqPreamp = m_txEqPreamp->GetValue();
	m_parameters->m_txEqGain0  = m_txEqGain0->GetValue();
	m_parameters->m_txEqGain1  = m_txEqGain1->GetValue();
	m_parameters->m_txEqGain2  = m_txEqGain2->GetValue();

	if (IsModal()) {
		EndModal(wxID_OK);
	} else {
		SetReturnCode(wxID_OK);
		Show(false);
	}
}

void CUWSDRPreferences::onHelp(wxCommandEvent& WXUNUSED(event))
{
	int page = m_noteBook->GetSelection();
	if (page == -1)
		return;

	::wxGetApp().showHelp(401 + page);
}

wxPanel* CUWSDRPreferences::createFrequencyTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the frequency shifts and offset."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(2);

	wxStaticText* label5 = new wxStaticText(panel, -1, _("Shift 1 (kHz)"));
	sizer->Add(label5, 0, wxALL, BORDER_SIZE);

	m_freqShift1 = new wxTextCtrl(panel, -1);
	sizer->Add(m_freqShift1, 0, wxALL, BORDER_SIZE);

	wxStaticText* label6 = new wxStaticText(panel, -1, _("Shift 2 (kHz)"));
	sizer->Add(label6, 0, wxALL, BORDER_SIZE);

	m_freqShift2 = new wxTextCtrl(panel, -1);
	sizer->Add(m_freqShift2, 0, wxALL, BORDER_SIZE);

	wxStaticText* label7 = new wxStaticText(panel, -1, _("Offset (Hz)"));
	sizer->Add(label7, 0, wxALL, BORDER_SIZE);

	m_freqOffset = new wxTextCtrl(panel, -1);
	sizer->Add(m_freqOffset, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxPanel* CUWSDRPreferences::createModeTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the filter, tuning and AGC/Deviation for the modes."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(4);

	wxStaticText* labelA = new wxStaticText(panel, -1, _("Mode"));
	sizer->Add(labelA, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelB = new wxStaticText(panel, -1, _("Filter"));
	sizer->Add(labelB, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelC = new wxStaticText(panel, -1, _("Tuning"));
	sizer->Add(labelC, 0, wxALL, BORDER_SIZE);

	wxStaticText* labelD = new wxStaticText(panel, -1, _("AGC/Deviation"));
	sizer->Add(labelD, 0, wxALL, BORDER_SIZE);

	wxStaticText* label1 = new wxStaticText(panel, -1, _("AM"));
	sizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_filterAM = createFilterChoice(panel);
	sizer->Add(m_filterAM, 0, wxALL, BORDER_SIZE);

	m_tuningAM = createTuningChoice(panel);
	sizer->Add(m_tuningAM, 0, wxALL, BORDER_SIZE);

	m_agcAM = createAGCChoice(panel);
	sizer->Add(m_agcAM, 0, wxALL, BORDER_SIZE);

	wxStaticText* label2 = new wxStaticText(panel, -1, _("CW Narrow"));
	sizer->Add(label2, 0, wxALL, BORDER_SIZE);

	m_filterCWN = createFilterChoice(panel);
	sizer->Add(m_filterCWN, 0, wxALL, BORDER_SIZE);

	m_tuningCWN = createTuningChoice(panel);
	sizer->Add(m_tuningCWN, 0, wxALL, BORDER_SIZE);

	m_agcCW = createAGCChoice(panel);
	sizer->Add(m_agcCW, 0, wxALL, BORDER_SIZE);

	wxStaticText* label3 = new wxStaticText(panel, -1, _("CW Wide"));
	sizer->Add(label3, 0, wxALL, BORDER_SIZE);

	m_filterCWW = createFilterChoice(panel);
	sizer->Add(m_filterCWW, 0, wxALL, BORDER_SIZE);

	m_tuningCWW = createTuningChoice(panel);
	sizer->Add(m_tuningCWW, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy3 = new wxStaticText(panel, -1, wxEmptyString);
	sizer->Add(dummy3, 0, wxALL, BORDER_SIZE);

	wxStaticText* label4 = new wxStaticText(panel, -1, _("Digital"));
	sizer->Add(label4, 0, wxALL, BORDER_SIZE);

	m_filterDig = createFilterChoice(panel);
	sizer->Add(m_filterDig, 0, wxALL, BORDER_SIZE);

	m_tuningDig = createTuningChoice(panel);
	sizer->Add(m_tuningDig, 0, wxALL, BORDER_SIZE);

	m_agcDig = createAGCChoice(panel);
	sizer->Add(m_agcDig, 0, wxALL, BORDER_SIZE);

	wxStaticText* label5 = new wxStaticText(panel, -1, _("FM Narrow"));
	sizer->Add(label5, 0, wxALL, BORDER_SIZE);

	m_filterFMN = createFilterChoice(panel);
	sizer->Add(m_filterFMN, 0, wxALL, BORDER_SIZE);

 	m_tuningFM = createTuningChoice(panel);
	sizer->Add(m_tuningFM, 0, wxALL, BORDER_SIZE);

	m_deviationFMN = createDeviationChoice(panel);
	sizer->Add(m_deviationFMN, 0, wxALL, BORDER_SIZE);

	wxStaticText* label6 = new wxStaticText(panel, -1, _("FM Wide"));
	sizer->Add(label6, 0, wxALL, BORDER_SIZE);

	m_filterFMW = createFilterChoice(panel);
	sizer->Add(m_filterFMW, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy6 = new wxStaticText(panel, -1, wxEmptyString);
	sizer->Add(dummy6, 0, wxALL, BORDER_SIZE);

	m_deviationFMW = createDeviationChoice(panel);
	sizer->Add(m_deviationFMW, 0, wxALL, BORDER_SIZE);

	wxStaticText* label7 = new wxStaticText(panel, -1, _("SSB"));
	sizer->Add(label7, 0, wxALL, BORDER_SIZE);

	m_filterSSB = createFilterChoice(panel);
	sizer->Add(m_filterSSB, 0, wxALL, BORDER_SIZE);

	m_tuningSSB = createTuningChoice(panel);
	sizer->Add(m_tuningSSB, 0, wxALL, BORDER_SIZE);

	m_agcSSB = createAGCChoice(panel);
	sizer->Add(m_agcSSB, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxChoice* CUWSDRPreferences::createAGCChoice(wxPanel* panel)
{
	wxChoice* agc = new wxChoice(panel, -1, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	agc->Append(_("Fast"));
	agc->Append(_("Medium"));
	agc->Append(_("Slow"));
	agc->Append(_("None"));

	return agc;
}

wxChoice* CUWSDRPreferences::createDeviationChoice(wxPanel* panel)
{
	wxChoice* dev = new wxChoice(panel, -1, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	dev->Append(_("6 kHz"));
	dev->Append(_("5 kHz"));
	dev->Append(_("3 kHz"));
	dev->Append(_("2.5 kHz"));
	dev->Append(_("2 kHz"));

	return dev;
}

wxChoice* CUWSDRPreferences::createFilterChoice(wxPanel* panel)
{
	wxChoice* filter = new wxChoice(panel, -1, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	filter->Append(wxT("20.0 kHz"));
	filter->Append(wxT("15.0 kHz"));
	filter->Append(wxT("10.0 kHz"));
	filter->Append(wxT("6.0 kHz"));
	filter->Append(wxT("4.0 kHz"));
	filter->Append(wxT("2.6 kHz"));
	filter->Append(wxT("2.1 kHz"));
	filter->Append(wxT("1.0 kHz"));
	filter->Append(wxT("500 Hz"));
	filter->Append(wxT("250 Hz"));
	filter->Append(wxT("100 Hz"));
	filter->Append(wxT("50 Hz"));
	filter->Append(wxT("25 Hz"));

	return filter;
}

wxChoice* CUWSDRPreferences::createTuningChoice(wxPanel* panel)
{
	wxChoice* tuning = new wxChoice(panel, -1, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	tuning->Append(_("Very fast"));
	tuning->Append(_("Fast"));
	tuning->Append(_("Medium"));
	tuning->Append(_("Slow"));
	tuning->Append(_("Very slow"));

	return tuning;
}

wxPanel* CUWSDRPreferences::createStepTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the step sizes."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(2);

	wxStaticText* label1 = new wxStaticText(panel, -1, _("Very fast (Hz/step)"));
	sizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_stepVeryFast = new wxTextCtrl(panel, -1);
	sizer->Add(m_stepVeryFast, 0, wxALL, BORDER_SIZE);

	wxStaticText* label2 = new wxStaticText(panel, -1, _("Fast (Hz/step)"));
	sizer->Add(label2, 0, wxALL, BORDER_SIZE);

	m_stepFast = new wxTextCtrl(panel, -1);
	sizer->Add(m_stepFast, 0, wxALL, BORDER_SIZE);

	wxStaticText* label3 = new wxStaticText(panel, -1, _("Medium (Hz/step)"));
	sizer->Add(label3, 0, wxALL, BORDER_SIZE);

	m_stepMedium = new wxTextCtrl(panel, -1);
	sizer->Add(m_stepMedium, 0, wxALL, BORDER_SIZE);

	wxStaticText* label4 = new wxStaticText(panel, -1, _("Slow (Hz/step)"));
	sizer->Add(label4, 0, wxALL, BORDER_SIZE);

	m_stepSlow = new wxTextCtrl(panel, -1);
	sizer->Add(m_stepSlow, 0, wxALL, BORDER_SIZE);

	wxStaticText* label5 = new wxStaticText(panel, -1, _("Very slow (Hz/step)"));
	sizer->Add(label5, 0, wxALL, BORDER_SIZE);

	m_stepVerySlow = new wxTextCtrl(panel, -1);
	sizer->Add(m_stepVerySlow, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxPanel* CUWSDRPreferences::createReceiveTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the parameters for the receiver."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(3);

	wxStaticText* nbLabel = new wxStaticText(panel, -1, _("Impulse noise blanker"));
	sizer->Add(nbLabel, 0, wxALL, BORDER_SIZE);

	m_nbButton = new wxCheckBox(panel, -1, wxEmptyString);
	sizer->Add(m_nbButton, 0, wxALL, BORDER_SIZE);

	m_nbValue = new wxSlider(panel, -1, 1, 1, 200, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_nbValue, 0, wxALL, BORDER_SIZE);

	wxStaticText* nb2Label = new wxStaticText(panel, -1, _("Mean noise blanker"));
	sizer->Add(nb2Label, 0, wxALL, BORDER_SIZE);

	m_nb2Button = new wxCheckBox(panel, -1, wxEmptyString);
	sizer->Add(m_nb2Button, 0, wxALL, BORDER_SIZE);

	m_nb2Value = new wxSlider(panel, -1, 1, 1, 1000, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_nb2Value, 0, wxALL, BORDER_SIZE);

	wxStaticText* binLabel = new wxStaticText(panel, -1, _("Binaural/Pan"));
	sizer->Add(binLabel, 0, wxALL, BORDER_SIZE);

	m_binButton = new wxCheckBox(panel, -1, wxEmptyString);
	sizer->Add(m_binButton, 0, wxALL, BORDER_SIZE);

	m_panValue = new wxSlider(panel, -1, 0, -100, 100, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_panValue, 0, wxALL, BORDER_SIZE);

	wxStaticText* rfLabel = new wxStaticText(panel, -1, _("RF gain (dB)"));
	sizer->Add(rfLabel, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy1 = new wxStaticText(panel, -1, wxEmptyString);
	sizer->Add(dummy1, 0, wxALL, BORDER_SIZE);

	m_rfValue = new wxSlider(panel, RXRF_GAIN, 0, -30, 20, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_rfValue, 0, wxALL, BORDER_SIZE);

	wxStaticText* rawLabel1 = new wxStaticText(panel, -1, _("Record"));
	sizer->Add(rawLabel1, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy2 = new wxStaticText(panel, -1, wxEmptyString);
	sizer->Add(dummy2, 0, wxALL, BORDER_SIZE);

	m_recordType = new wxChoice(panel, -1, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1));
	m_recordType->Append(_("16-bit Audio (Mono)"));
	m_recordType->Append(_("16-bit Audio (Stereo)"));
	m_recordType->Append(_("32-bit Raw I and Q (Stereo)"));
	sizer->Add(m_recordType, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxPanel* CUWSDRPreferences::createTransmitTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the parameters for the transmitter, except ALC."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(3);

	wxStaticText* spLabel = new wxStaticText(panel, -1, _("Speech processor (dB)"));
	sizer->Add(spLabel, 0, wxALL, BORDER_SIZE);

	m_spButton = new wxCheckBox(panel, -1, wxEmptyString);
	sizer->Add(m_spButton, 0, wxALL, BORDER_SIZE);

	m_spValue = new wxSlider(panel, -1, 3, 0, 20, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_spValue, 0, wxALL, BORDER_SIZE);

	wxStaticText* carrierLabel = new wxStaticText(panel, -1, _("AM carrier level (%)"));
	sizer->Add(carrierLabel, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy1 = new wxStaticText(panel, -1, wxEmptyString);
	sizer->Add(dummy1, 0, wxALL, BORDER_SIZE);

	m_carrierLevel = new wxSlider(panel, -1, 100, 0, 100, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_carrierLevel, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxPanel* CUWSDRPreferences::createALCTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the parameters for the transmit ALC."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(2);

	wxStaticText* alcAttackLabel = new wxStaticText(panel, -1, _("ALC attack (ms)"));
	sizer->Add(alcAttackLabel, 0, wxALL, BORDER_SIZE);

	m_alcAttack = new wxSlider(panel, -1, 2, 1, 10, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_alcAttack, 0, wxALL, BORDER_SIZE);

	wxStaticText* alcDecayLabel = new wxStaticText(panel, -1, _("ALC decay (ms)"));
	sizer->Add(alcDecayLabel, 0, wxALL, BORDER_SIZE);

	m_alcDecay = new wxSlider(panel, -1, 10, 1, 50, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_alcDecay, 0, wxALL, BORDER_SIZE);

	wxStaticText* alcHangLabel = new wxStaticText(panel, -1, _("ALC hang (ms)"));
	sizer->Add(alcHangLabel, 0, wxALL, BORDER_SIZE);

	m_alcHang = new wxSlider(panel, -1, 500, 10, 5000, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_alcHang, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxPanel* CUWSDRPreferences::createIQTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the parameters for I/Q handling."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(4);

	wxStaticText* label1 = new wxStaticText(panel, -1, _("Receive phase"));
	sizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_rxIQPhase = new wxSpinCtrl(panel, RXIQ_PHASE, wxEmptyString, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));
	m_rxIQPhase->SetRange(-100, 100);
	sizer->Add(m_rxIQPhase, 0, wxALL, BORDER_SIZE);

	wxStaticText* label3 = new wxStaticText(panel, -1, _("Transmit phase"));
	sizer->Add(label3, 0, wxALL, BORDER_SIZE);

	m_txIQPhase = new wxSpinCtrl(panel, TXIQ_PHASE, wxEmptyString, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));
	m_txIQPhase->SetRange(-100, 100);
	sizer->Add(m_txIQPhase, 0, wxALL, BORDER_SIZE);

	wxStaticText* label2 = new wxStaticText(panel, -1, _("Receive gain"));
	sizer->Add(label2, 0, wxALL, BORDER_SIZE);

	m_rxIQGain = new wxSpinCtrl(panel, RXIQ_GAIN, wxEmptyString, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));
	m_rxIQGain->SetRange(-100, 100);
	sizer->Add(m_rxIQGain, 0, wxALL, BORDER_SIZE);

	wxStaticText* label4 = new wxStaticText(panel, -1, _("Transmit gain"));
	sizer->Add(label4, 0, wxALL, BORDER_SIZE);

	m_txIQGain = new wxSpinCtrl(panel, TXIQ_GAIN, wxEmptyString, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));
	m_txIQGain->SetRange(-100, 100);
	sizer->Add(m_txIQGain, 0, wxALL, BORDER_SIZE);

	wxStaticText* label5 = new wxStaticText(panel, -1, _("Method"));
	sizer->Add(label5, 0, wxALL, BORDER_SIZE);

	m_method = new wxChoice(panel, -1, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));
	m_method->Append(_("Hilbert"));
	m_method->Append(_("Weaver"));
	sizer->Add(m_method, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

wxPanel* CUWSDRPreferences::createTXEQTab(wxNotebook* noteBook)
{
	wxPanel* panel = new wxPanel(noteBook, -1);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxStaticText* labelZ = new wxStaticText(panel, -1, _("Set the parameters for transmit audio equaliser."));
	mainSizer->Add(labelZ, 0, wxALL, BORDER_SIZE);

	wxFlexGridSizer* sizer = new wxFlexGridSizer(2);

	wxStaticText* label1 = new wxStaticText(panel, -1, _("Enabled"));
	sizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_txEqEnable = new wxChoice(panel, TXEQ_ENABLE, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));
	m_txEqEnable->Append(_("Off"));
	m_txEqEnable->Append(_("On"));
	sizer->Add(m_txEqEnable, 0, wxALL, BORDER_SIZE);

	wxStaticText* label2 = new wxStaticText(panel, -1, _("Preamp gain (dB)"));
	sizer->Add(label2, 0, wxALL, BORDER_SIZE);

	m_txEqPreamp = new wxSlider(panel, TXEQ_PREAMP, 0, -10, 10, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_txEqPreamp, 0, wxALL, BORDER_SIZE);

	wxStaticText* label3 = new wxStaticText(panel, -1, _("Gain < 400Hz (dB)"));
	sizer->Add(label3, 0, wxALL, BORDER_SIZE);

	m_txEqGain0 = new wxSlider(panel, TXEQ_GAIN0, 0, -10, 10, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_txEqGain0, 0, wxALL, BORDER_SIZE);

	wxStaticText* label4 = new wxStaticText(panel, -1, _("Gain 400-1500Hz (dB)"));
	sizer->Add(label4, 0, wxALL, BORDER_SIZE);

	m_txEqGain1 = new wxSlider(panel, TXEQ_GAIN1, 0, -10, 10, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_txEqGain1, 0, wxALL, BORDER_SIZE);

	wxStaticText* label5 = new wxStaticText(panel, -1, _("Gain > 1500Hz (dB)"));
	sizer->Add(label5, 0, wxALL, BORDER_SIZE);

	m_txEqGain2 = new wxSlider(panel, TXEQ_GAIN2, 0, -10, 10, wxDefaultPosition, wxSize(SLIDER_WIDTH, -1), wxSL_HORIZONTAL | wxSL_LABELS | wxSL_BOTTOM);
	sizer->Add(m_txEqGain2, 0, wxALL, BORDER_SIZE);

	mainSizer->Add(sizer, 0, wxTOP, BORDER_SIZE);

	panel->SetAutoLayout(true);

	sizer->Fit(panel);
	sizer->SetSizeHints(panel);

	panel->SetSizer(mainSizer);

	return panel;
}

void CUWSDRPreferences::onIQChanged(wxSpinEvent& event)
{
	switch (event.GetId()) {
		case RXIQ_PHASE:
		case RXIQ_GAIN: {
			int phase = m_rxIQPhase->GetValue();
			int gain  = m_rxIQGain->GetValue();
			m_dsp->setRXIAndQ(phase, gain);
			break;
		}

		case TXIQ_PHASE:
		case TXIQ_GAIN: {
			int phase = m_txIQPhase->GetValue();
			int gain  = m_txIQGain->GetValue();
			m_dsp->setTXIAndQ(phase, gain);
			break;
		}
	}
}

void CUWSDRPreferences::onRFGainChanged(wxScrollEvent& WXUNUSED(event))
{
	double gainDb = double(m_rfValue->GetValue());

	// Map -30 -> 20 to 1 -> 100000
	unsigned int gain = (unsigned int)(1000.0 * ::pow(10.0, gainDb / 10.0) + 0.5);

	m_dsp->setRFGain(gain);
}

void CUWSDRPreferences::onTXEQEnabled(wxCommandEvent& WXUNUSED(event))
{
	bool enabled = m_txEqEnable->GetSelection() == 1;

	int values[4U];
	values[0U] = m_txEqPreamp->GetValue();
	values[1U] = m_txEqGain0->GetValue();
	values[2U] = m_txEqGain1->GetValue();
	values[3U] = m_txEqGain2->GetValue();

	m_dsp->setEQ(enabled);
	m_dsp->setEQLevels(4U, values);
}

void CUWSDRPreferences::onTXEQChanged(wxScrollEvent& WXUNUSED(event))
{
	bool enabled = m_txEqEnable->GetSelection() == 1;

	int values[4U];
	values[0U] = m_txEqPreamp->GetValue();
	values[1U] = m_txEqGain0->GetValue();
	values[2U] = m_txEqGain1->GetValue();
	values[3U] = m_txEqGain2->GetValue();

	m_dsp->setEQ(enabled);
	m_dsp->setEQLevels(4U, values);
}
