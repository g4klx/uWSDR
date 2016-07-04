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
#include "UWSDRFrame.h"
#include "UWSDRApp.h"
#include "UWSDRDefs.h"
#include "NullController.h"
#include "SI570Controller.h"
#include "HackRFController.h"
#include "FreqKeypad.h"
#include "NullReader.h"
#include "NullWriter.h"
#include "SignalReader.h"
#include "TwoToneReader.h"
#include "ThreeToneReader.h"
#include "SoundFileReader.h"
#include "SoundFileWriter.h"
#include "JackReaderWriter.h"
#include "SoundCardReaderWriter.h"
#include "GriffinPowerMate.h"
#include "ExternalProtocolHandler.h"

#if defined(__WXGTK__) || defined(__WXMAC__)
#include "UWSDR.xpm"
#endif

#include <wx/aboutdlg.h>


enum {
	MENU_KEYPAD = 36427,
	MENU_PREFERENCES,
	MENU_CW_KEYBOARD,
	MENU_VOICE_KEYBOARD,
	MENU_RECORD,
	MENU_HARDWARE_INFO,
	MENU_ABOUT,
	MENU_EXIT
};

enum {
	MENU_BUTTON = 17856,
	VFO_AB_BUTTON,
	VFO_CD_BUTTON,
	VFO_SWAP_BUTTON,
	VFO_SPLIT_BUTTON,
	VFO_SHIFT1_BUTTON,
	VFO_SHIFT2_BUTTON,
	FREQ_KNOB,
	FREQ_MHZ1_BUTTON,
	FREQ_MHZ2_BUTTON,
	FREQ_DISPLAY,
	INFO_DISPLAY,
	FREQ_SPECTRUM,
	MODE_CHOICE,
	FILTER_CHOICE,
	RIT_BUTTON,
	MUTE_BUTTON,
	RIT_KNOB,
	TX_BUTTON,
	SMETER,
	MIC_KNOB,
	POWER_KNOB,
	VOLUME_KNOB,
	SQUELCH_KNOB,
	DISPLAY_TIMER,
	TUNING_HARDWARE
};

DEFINE_EVENT_TYPE(TRANSMIT_ON_EVENT)
DEFINE_EVENT_TYPE(TRANSMIT_OFF_EVENT)
DEFINE_EVENT_TYPE(KEY_ON_EVENT)
DEFINE_EVENT_TYPE(KEY_OFF_EVENT)
DEFINE_EVENT_TYPE(COMMAND_NAK_EVENT)
DEFINE_EVENT_TYPE(COMMAND_ERROR_EVENT)
DEFINE_EVENT_TYPE(CONNECTION_LOST_EVENT)
DEFINE_EVENT_TYPE(TUNING_EVENT)

BEGIN_EVENT_TABLE(CUWSDRFrame, wxFrame)
	EVT_BUTTON(MENU_BUTTON, CUWSDRFrame::onMenuButton)
	EVT_MENU(MENU_KEYPAD, CUWSDRFrame::onMenuSelection)
	EVT_MENU(MENU_CW_KEYBOARD, CUWSDRFrame::onMenuSelection)
	EVT_MENU(MENU_VOICE_KEYBOARD, CUWSDRFrame::onMenuSelection)
	EVT_MENU(MENU_PREFERENCES, CUWSDRFrame::onMenuSelection)
	EVT_MENU(MENU_RECORD, CUWSDRFrame::onMenuSelection)
	EVT_MENU(wxID_HELP, CUWSDRFrame::onMenuSelection)
	EVT_MENU(MENU_HARDWARE_INFO, CUWSDRFrame::onMenuSelection)
	EVT_MENU(wxID_ABOUT, CUWSDRFrame::onMenuSelection)
	EVT_MENU(wxID_EXIT, CUWSDRFrame::onMenuSelection)
	EVT_BUTTON(VFO_AB_BUTTON, CUWSDRFrame::onVFOButton)
	EVT_BUTTON(VFO_CD_BUTTON, CUWSDRFrame::onVFOButton)
	EVT_BUTTON(VFO_SWAP_BUTTON, CUWSDRFrame::onVFOButton)
	EVT_BUTTON(VFO_SPLIT_BUTTON, CUWSDRFrame::onVFOButton)
	EVT_BUTTON(VFO_SHIFT1_BUTTON, CUWSDRFrame::onVFOButton)
	EVT_BUTTON(VFO_SHIFT2_BUTTON, CUWSDRFrame::onVFOButton)
	EVT_BUTTON(FREQ_MHZ1_BUTTON, CUWSDRFrame::onMHzButton)
	EVT_BUTTON(FREQ_MHZ2_BUTTON, CUWSDRFrame::onMHzButton)
	EVT_CHOICE(MODE_CHOICE, CUWSDRFrame::onModeChoice)
	EVT_CHOICE(FILTER_CHOICE, CUWSDRFrame::onFilterChoice)
	EVT_TOGGLEBUTTON(RIT_BUTTON, CUWSDRFrame::onRITButton)
	EVT_TOGGLEBUTTON(MUTE_BUTTON, CUWSDRFrame::onMuteButton)
	EVT_TOGGLEBUTTON(TX_BUTTON, CUWSDRFrame::onTXButton)
	EVT_MOUSEWHEEL(CUWSDRFrame::onMouseWheel)
	EVT_TIMER(DISPLAY_TIMER, CUWSDRFrame::onTimer)
	EVT_CLOSE(CUWSDRFrame::onClose)
	EVT_CUSTOM(TRANSMIT_ON_EVENT, wxID_ANY, CUWSDRFrame::onTransmitOn)
	EVT_CUSTOM(TRANSMIT_OFF_EVENT, wxID_ANY, CUWSDRFrame::onTransmitOff)
	EVT_CUSTOM(KEY_ON_EVENT, wxID_ANY, CUWSDRFrame::onKeyOn)
	EVT_CUSTOM(KEY_OFF_EVENT, wxID_ANY, CUWSDRFrame::onKeyOff)
	EVT_CUSTOM(COMMAND_NAK_EVENT, wxID_ANY, CUWSDRFrame::onCommandNak)
	EVT_CUSTOM(COMMAND_ERROR_EVENT, wxID_ANY, CUWSDRFrame::onCommandError)
	EVT_CUSTOM(CONNECTION_LOST_EVENT, wxID_ANY, CUWSDRFrame::onConnectionLost)
	EVT_CUSTOM(TUNING_EVENT, wxID_ANY, CUWSDRFrame::onTune)
END_EVENT_TABLE()

const unsigned int BLOCK_SIZE      = 2048U;

CUWSDRFrame::CUWSDRFrame(const wxString& title) :
wxFrame(NULL, -1, title),
m_timer(),
m_parameters(NULL),
m_dsp(NULL),
m_sdr(NULL),
m_tuning(NULL),
m_rxOn(true),
m_txIntOn(0U),
m_txExtOn(false),
m_stepSize(0.0),
m_record(false),
m_txInRange(false),
m_menu(NULL),
m_swap(NULL),
m_split(NULL),
m_shift1(NULL),
m_shift2(NULL),
m_mhzMinus(NULL),
m_mhzPlus(NULL),
m_freqDisplay(NULL),
m_spectrumDisplay(NULL),
m_infoBox(NULL),
m_mode(NULL),
m_filter(NULL),
m_ritCtrl(NULL),
m_rit(NULL),
m_mute(NULL),
m_transmit(NULL),
m_sMeter(NULL),
m_micGain(NULL),
m_power(NULL),
m_afGain(NULL),
m_squelch(NULL),
m_spectrum(NULL),
m_frequency(),
m_voiceKeyboard(NULL),
m_cwKeyboard(NULL)
{
	SetIcon(wxICON(UWSDR));

	m_spectrum = new float[SPECTRUM_SIZE];

	// The top level sizer, graph, list box and then controls
	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	createMenu();

	wxPanel* panel = new wxPanel(this);

	wxBoxSizer* panelSizer = new wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* freqDialSizer = new wxBoxSizer(wxVERTICAL);
	freqDialSizer->Add(createMenuButton(panel), 0, wxTOP | wxLEFT | wxRIGHT, BORDER_SIZE);
	freqDialSizer->Add(createVFOButtons(panel), 0,wxALL, BORDER_SIZE);
	freqDialSizer->Add(createFreqDial(panel), 0, wxALL, BORDER_SIZE);
	freqDialSizer->Add(createMHzButtons(panel), 0, wxALL, BORDER_SIZE);
	panelSizer->Add(freqDialSizer, 0, wxLEFT | wxRIGHT, BORDER_SIZE);

	wxBoxSizer* freqSizer = new wxBoxSizer(wxVERTICAL);

	wxBoxSizer* freqInfoSizer = new wxBoxSizer(wxHORIZONTAL);
	freqInfoSizer->Add(createFreqDisplay(panel), 0, wxTOP | wxBOTTOM, BORDER_SIZE);
	freqInfoSizer->Add(createInfoBox(panel), 0, wxTOP | wxBOTTOM, BORDER_SIZE);
	freqSizer->Add(freqInfoSizer, 0, wxBOTTOM | wxRIGHT, BORDER_SIZE);

	freqSizer->Add(createSpectrumDisplay(panel), 0, wxBOTTOM | wxRIGHT, BORDER_SIZE);

	wxFlexGridSizer* belowFreqSizer = new wxFlexGridSizer(3);

	wxGridSizer* bandModeSizer = new wxGridSizer(2, 0, 8);
	bandModeSizer->Add(createModeChoice(panel));
	bandModeSizer->Add(createRITButton(panel), 0, wxEXPAND);
	bandModeSizer->Add(createFilterChoice(panel));
	bandModeSizer->Add(createMuteButton(panel), 0, wxEXPAND);

	belowFreqSizer->Add(bandModeSizer, 0, wxEXPAND |wxLEFT | wxRIGHT, BORDER_SIZE);
	belowFreqSizer->Add(createRIT(panel), 0, wxLEFT | wxRIGHT, BORDER_SIZE);
	belowFreqSizer->Add(createTransmitButton(panel), 0, wxLEFT, BORDER_SIZE);

	freqSizer->Add(belowFreqSizer, 0, wxRIGHT | wxBOTTOM, BORDER_SIZE);

	panelSizer->Add(freqSizer, 0, wxLEFT, BORDER_SIZE);

	wxBoxSizer* sMeterKnobSizer = new wxBoxSizer(wxVERTICAL);
	sMeterKnobSizer->Add(createSMeter(panel), 0, wxBOTTOM, BORDER_SIZE);

	wxGridSizer* knobSizer = new wxGridSizer(2, 0, 3);
	knobSizer->Add(createMicGain(panel));
	knobSizer->Add(createPowerControl(panel));
	knobSizer->Add(createAFGain(panel));
	knobSizer->Add(createSquelch(panel));

	sMeterKnobSizer->Add(knobSizer);

	panelSizer->Add(sMeterKnobSizer, 0, wxALL, BORDER_SIZE);

	panel->SetSizer(panelSizer);
	panelSizer->SetSizeHints(panel);

	mainSizer->Add(panel);

	SetSizer(mainSizer);
	mainSizer->SetSizeHints(this);

	m_voiceKeyboard = new CVoiceKeyboard(this, -1);
	m_cwKeyboard    = new CCWKeyboard(this, -1);
}

CUWSDRFrame::~CUWSDRFrame()
{
	delete[] m_spectrum;
	delete   m_menu;
	delete   m_voiceKeyboard;
	delete   m_cwKeyboard;
}

void CUWSDRFrame::setParameters(CSDRParameters* parameters)
{
	wxASSERT(parameters != NULL);

	m_parameters = parameters;

	CExternalProtocolHandler* extHandler = NULL;
	if (!m_parameters->m_externalName.IsEmpty())
		extHandler = new CExternalProtocolHandler(m_parameters->m_hardwareSampleRate, BLOCK_SIZE, m_parameters->m_externalName, m_parameters->m_externalAddrs, this);

	switch (m_parameters->m_tuning) {
		case TUNINGHW_POWERMATE:
			m_tuning = new CGriffinPowerMate();
			break;
		default:
			break;
	}

	if (m_tuning != NULL) {
		m_tuning->setCallback(this, TUNING_HARDWARE);

		bool ret = m_tuning->open();
		if (!ret) {
			wxLogError(wxT("Problems communicating with the tuning hardware"));
			::wxMessageBox(_("Problems communicating with the tuning hardware"), _("UWSDR Error"), wxICON_ERROR);
			Close(true);
			return;
		}
	}

	switch (m_parameters->m_hardwareType) {
		case TYPE_SI570TXRX:
		case TYPE_SI570RX:
			m_sdr = new CSI570Controller(m_parameters->m_hardwareFreqMult);
			break;
		case TYPE_HACKRF:
			m_sdr = new CHackRFController;
			break;
		default:
			m_sdr = new CNullController;
			break;
	}

	m_sdr->setCallback(this);

	bool ret = m_sdr->open();
	if (!ret) {
		wxLogError(wxT("Problems communicating with the SDR"));
		::wxMessageBox(_("Problems communicating with the SDR"), _("UWSDR Error"), wxICON_ERROR);
		Close(true);
		return;
	}

	if (!m_parameters->m_hardwareTXRange.isEmpty())
		m_sdr->enableTX(true);
	m_sdr->enableRX(true);

	m_spectrumDisplay->setSampleRate(m_parameters->m_hardwareSampleRate);
	m_spectrumDisplay->setType(m_parameters->m_spectrumType);
	m_spectrumDisplay->setSpeed(m_parameters->m_spectrumSpeed);
	m_spectrumDisplay->setDB(m_parameters->m_spectrumDB);
	m_spectrumDisplay->setBandwidth(m_parameters->m_hardwareSampleRate);

	m_freqDisplay->setMaxFrequency(m_parameters->m_hardwareMaxFreq);

	m_dsp = new CDSPControl(m_parameters->m_hardwareSampleRate, m_parameters->m_hardwareReceiveGainOffset, BLOCK_SIZE, m_parameters->m_hardwareSwapIQ);

	switch (m_parameters->m_hardwareType) {
		case TYPE_SI570RX:
			// TX is disabled, RX is from audio card for signal input and audio output
			m_dsp->setTXReader(new CNullReader());
			m_dsp->setTXWriter(new CNullWriter());

			// TX and RX are from audio cards for signal input and audio output, also simple TX/RX control
			if (m_parameters->m_sdrAudioType == SOUND_JACK) {
				CJackReaderWriter* rw = new CJackReaderWriter(m_parameters->m_name + wxT(" SDR"), 2U, 0U);
				m_dsp->setRXReader(rw);
			} else {
				CSoundCardReaderWriter* rw = new CSoundCardReaderWriter(m_parameters->m_sdrAudioInDev, -1, 2U, 0U);
				m_dsp->setRXReader(rw);
			}

			if (m_parameters->m_userAudioType == SOUND_JACK) {
				CJackReaderWriter* rw = new CJackReaderWriter(m_parameters->m_name + wxT(" User"), 0U, 2U);
				m_dsp->setRXWriter(rw);
			} else {
				CSoundCardReaderWriter* rw = new CSoundCardReaderWriter(-1, m_parameters->m_userAudioOutDev, 0U, 2U);
				m_dsp->setRXWriter(rw);
			}
			break;

		case TYPE_SI570TXRX:
			// TX and RX are from audio cards for signal input and audio output, also simple TX/RX control
			if (m_parameters->m_sdrAudioType == SOUND_JACK) {
				CJackReaderWriter* rw = new CJackReaderWriter(m_parameters->m_name + wxT(" SDR"), 2U, 2U);
				m_dsp->setRXReader(rw);
				m_dsp->setTXWriter(rw);
			} else {
				CSoundCardReaderWriter* rw = new CSoundCardReaderWriter(m_parameters->m_sdrAudioInDev, m_parameters->m_sdrAudioOutDev, 2U, 2U);
				m_dsp->setRXReader(rw);
				m_dsp->setTXWriter(rw);
			}

			if (m_parameters->m_userAudioType == SOUND_JACK) {
				CJackReaderWriter* rw = new CJackReaderWriter(m_parameters->m_name + wxT(" User"), 2U, 2U);
				m_dsp->setTXReader(rw);
				m_dsp->setRXWriter(rw);
			} else {
				CSoundCardReaderWriter* rw = new CSoundCardReaderWriter(m_parameters->m_userAudioInDev, m_parameters->m_userAudioOutDev, 2U, 2U);
				m_dsp->setTXReader(rw);
				m_dsp->setRXWriter(rw);
			}
			break;

		case TYPE_DEMO:
			// A self contained variant for demo's and testing
			if (m_parameters->m_userAudioType == SOUND_JACK) {
				CJackReaderWriter* rw = new CJackReaderWriter(m_parameters->m_name + wxT(" User"), 2U, 2U);
				m_dsp->setTXReader(new CThreeToneReader(500.0F, 1500.0F, 2000.0F, 0.25F, rw));
				m_dsp->setRXWriter(rw);
				// m_dsp->setTXReader(rw);
			} else {
				CSoundCardReaderWriter* rw = new CSoundCardReaderWriter(m_parameters->m_userAudioInDev, m_parameters->m_userAudioOutDev, 2U, 2U);
				m_dsp->setTXReader(new CThreeToneReader(500.0F, 1500.0F, 2000.0F, 0.25F, rw));
				m_dsp->setRXWriter(rw);
				// m_dsp->setTXReader(rw);
			}

			m_dsp->setTXWriter(new CNullWriter());
			m_dsp->setRXReader(new CSignalReader(1000.5F, 0.00003F, 0.00004F, 0.0F, 0.0F, m_parameters->m_hardwareSwapIQ));
			break;

		case TYPE_HACKRF:
//			wxASSERT(hpsdr != NULL);
//
//			m_dsp->setTXReader(new CHPSDRAudioReader(hpsdr));
//			m_dsp->setTXWriter(new CHPSDRDataWriter(hpsdr));
//
//			m_dsp->setRXReader(new CHPSDRDataReader(hpsdr));
//			m_dsp->setRXWriter(new CHPSDRAudioWriter(hpsdr));
//
			if (m_parameters->m_userAudioType == SOUND_JACK) {
				CJackReaderWriter* rw = new CJackReaderWriter(m_parameters->m_name + wxT(" User"), 2U, 2U);
				if (m_parameters->m_hardwareTXRange.isEmpty())
					m_dsp->setTXReader(new CNullReader());
				else
					m_dsp->setTXReader(rw);
				m_dsp->setRXWriter(rw);
			} else {
				CSoundCardReaderWriter* rw = new CSoundCardReaderWriter(m_parameters->m_userAudioInDev, m_parameters->m_userAudioOutDev, 2U, 2U);
				if (m_parameters->m_hardwareTXRange.isEmpty())
					m_dsp->setTXWriter(new CNullWriter());
				else
					m_dsp->setTXWriter(rw);
				m_dsp->setRXWriter(rw);
			}
			break;
	}

	m_infoBox->setVFO(m_parameters->m_vfoChoice);
	m_infoBox->setSplitShift(m_parameters->m_vfoSplitShift);
	m_infoBox->setRIT(m_parameters->m_ritOn);

	m_mode->SetSelection(m_parameters->m_mode);

	m_filter->SetSelection(m_parameters->m_filter);

	m_ritCtrl->SetValue(m_parameters->m_ritOn);
	m_ritCtrl->SetLabel(m_parameters->m_ritOn ? _("Off") : _("On"));
	m_rit->setValue(m_parameters->m_ritFreq);

	m_afGain->setValue(m_parameters->m_afGain);
	m_dsp->setAFGain(m_parameters->m_afGain);

	m_dsp->setRFGain(m_parameters->m_rfGain);

	if (extHandler != NULL)
		m_dsp->setExternalHandler(extHandler);

	m_squelch->setValue(m_parameters->m_squelch);
	m_dsp->setSquelch(m_parameters->m_squelch);

	m_dsp->setNB(m_parameters->m_nbOn);
	m_dsp->setNBValue(m_parameters->m_nbValue);
	m_dsp->setNB2(m_parameters->m_nb2On);
	m_dsp->setNB2Value(m_parameters->m_nb2Value);

	m_dsp->setSP(m_parameters->m_spOn);
	m_dsp->setSPValue(m_parameters->m_spValue);

	int txEqVals[4U];
	txEqVals[0U] = m_parameters->m_txEqPreamp;
	txEqVals[1U] = m_parameters->m_txEqGain0;
	txEqVals[2U] = m_parameters->m_txEqGain1;
	txEqVals[3U] = m_parameters->m_txEqGain2;

	m_dsp->setEQ(m_parameters->m_txEqOn);
	m_dsp->setEQLevels(4U, txEqVals);

	m_dsp->setCarrierLevel(m_parameters->m_carrierLevel);

	m_dsp->setALCValue(m_parameters->m_alcAttack, m_parameters->m_alcDecay, m_parameters->m_alcHang);

	m_dsp->setRXIAndQ(m_parameters->m_rxIQphase, m_parameters->m_rxIQgain);
	m_dsp->setTXIAndQ(m_parameters->m_txIQphase, m_parameters->m_txIQgain);

	m_dsp->setBinaural(m_parameters->m_binaural);
	m_dsp->setPan(m_parameters->m_pan);

	m_sMeter->setRXMeter(m_parameters->m_rxMeter);
	m_sMeter->setTXMeter(m_parameters->m_txMeter);

	m_cwKeyboard->setSpeed(m_parameters->m_cwSpeed);
	m_cwKeyboard->setLocal(m_parameters->m_cwLocal);
	m_cwKeyboard->setRemote(m_parameters->m_cwRemote);
	m_cwKeyboard->setLocator(m_parameters->m_cwLocator);
	m_cwKeyboard->setReport(m_parameters->m_cwReport);
	m_cwKeyboard->setSerial(m_parameters->m_cwSerial);
	for (unsigned int i = 0; i < CWKEYBOARD_COUNT; i++)
		m_cwKeyboard->setMessage(i, m_parameters->m_cwMessage[i]);

	m_voiceKeyboard->setDir(m_parameters->m_voiceDir);
	for (unsigned int j = 0; j < VOICEKEYER_COUNT; j++)
		m_voiceKeyboard->setFile(j, m_parameters->m_voiceFile[j]);

	m_timer.SetOwner(this, DISPLAY_TIMER);
	m_timer.Start(200);

	// Mute only works with UWSDR hardware
	m_mute->Disable();

	normaliseFreq();

	normaliseMode();

	// Must be after normaliseMode
	m_dsp->setWeaver(m_parameters->m_weaver);

	normaliseFreq();

	ret = m_dsp->open();
	if (!ret) {
		wxLogError(wxT("Problems opening the I/O ports"));
		::wxMessageBox(_("Problems opening the input/output ports."), _("UWSDR Error"), wxICON_ERROR);
		Close(true);
		return;
	}
}

CSDRParameters* CUWSDRFrame::getParameters()
{
	return m_parameters;
}

void CUWSDRFrame::createMenu()
{
	m_menu = new wxMenu();

	m_menu->Append(MENU_KEYPAD,          _("Frequency Keypad..."));
	m_menu->Append(MENU_PREFERENCES,     _("Preferences..."));
	m_menu->Append(MENU_CW_KEYBOARD,     _("CW Keyboard..."));
	m_menu->Append(MENU_VOICE_KEYBOARD,  _("Voice Keyer..."));
	m_menu->AppendCheckItem(MENU_RECORD, _("Record"));
	m_menu->AppendSeparator();
	m_menu->Append(wxID_HELP,            _("Help\tF1"));
	m_menu->Append(MENU_HARDWARE_INFO,   _("Hardware Info"));
	m_menu->Append(wxID_ABOUT,           _("About UWSDR"));
	m_menu->AppendSeparator();
	m_menu->Append(wxID_EXIT,            _("Exit\tALT-F4"));
}

wxButton* CUWSDRFrame::createMenuButton(wxWindow* window)
{
	return new wxButton(window, MENU_BUTTON, _("Menu"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH, BUTTON_HEIGHT));
}

wxSizer* CUWSDRFrame::createVFOButtons(wxWindow* window)
{
	wxSizer* grid = new wxGridSizer(2);

	wxButton* vfoA = new wxButton(window, VFO_AB_BUTTON, _("VFO A/B"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(vfoA);

	wxButton* vfoB = new wxButton(window, VFO_CD_BUTTON, _("VFO C/D"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(vfoB);

	m_swap = new wxButton(window, VFO_SWAP_BUTTON, _("SWAP"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(m_swap);

	m_split = new wxButton(window, VFO_SPLIT_BUTTON, _("SPLIT"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(m_split);

	m_shift1 = new wxButton(window, VFO_SHIFT1_BUTTON, _("SHIFT 1"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(m_shift1);

	m_shift2 = new wxButton(window, VFO_SHIFT2_BUTTON, _("SHIFT 2"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(m_shift2);

	grid->SetSizeHints(window);

	return grid;
}

wxPanel* CUWSDRFrame::createFreqDial(wxWindow* window)
{
	CFreqDial* freqDial = new CFreqDial(window, FREQ_KNOB, this, wxDefaultPosition, wxSize(FREQDIAL_WIDTH, FREQDIAL_HEIGHT));

	return freqDial;
}

wxSizer* CUWSDRFrame::createMHzButtons(wxWindow* window)
{
	wxSizer* grid = new wxGridSizer(2);

	m_mhzMinus = new wxButton(window, FREQ_MHZ1_BUTTON, _("MHz -"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(m_mhzMinus);

	m_mhzPlus = new wxButton(window, FREQ_MHZ2_BUTTON, _("MHz +"), wxDefaultPosition, wxSize(FREQDIAL_WIDTH / 2, BUTTON_HEIGHT));
	grid->Add(m_mhzPlus);

	grid->SetSizeHints(window);

	return grid;
}

wxPanel* CUWSDRFrame::createFreqDisplay(wxWindow* window)
{
	m_freqDisplay = new CFreqDisplay(window, FREQ_DISPLAY, wxDefaultPosition, wxSize(FREQUENCY_WIDTH, FREQUENCY_HEIGHT));

	normaliseFreq();

	return m_freqDisplay;
}

wxPanel* CUWSDRFrame::createSpectrumDisplay(wxWindow* window)
{
	m_spectrumDisplay = new CSpectrumDisplay(window, FREQ_SPECTRUM, wxDefaultPosition, wxSize(SPECTRUM_WIDTH, SPECTRUM_HEIGHT));

	return m_spectrumDisplay;
}

wxSizer* CUWSDRFrame::createModeChoice(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Mode")), wxVERTICAL);

	m_mode = new wxChoice(window, MODE_CHOICE, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	m_mode->Append(_("AM"));
	m_mode->Append(_("CW (L) Narrow"));
	m_mode->Append(_("CW (L) Wide"));
	m_mode->Append(_("CW (U) Narrow"));
	m_mode->Append(_("CW (U) Wide"));
	m_mode->Append(_("Digital (L)"));
	m_mode->Append(_("Digital (U)"));
	m_mode->Append(_("FM Narrow"));
	m_mode->Append(_("FM Wide"));
	m_mode->Append(_("LSB"));
	m_mode->Append(_("USB"));

	sizer->Add(m_mode, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createFilterChoice(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Filter")), wxVERTICAL);

	m_filter = new wxChoice(window, FILTER_CHOICE, wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	m_filter->Append(wxT("20.0 kHz"));
	m_filter->Append(wxT("15.0 kHz"));
	m_filter->Append(wxT("10.0 kHz"));
	m_filter->Append(wxT("6.0 kHz"));
	m_filter->Append(wxT("4.0 kHz"));
	m_filter->Append(wxT("2.6 kHz"));
	m_filter->Append(wxT("2.1 kHz"));
	m_filter->Append(wxT("1.0 kHz"));
	m_filter->Append(wxT("500 Hz"));
	m_filter->Append(wxT("250 Hz"));
	m_filter->Append(wxT("100 Hz"));
	m_filter->Append(wxT("50 Hz"));
	m_filter->Append(wxT("25 Hz"));
	m_filter->Append(_("Auto"));

	sizer->Add(m_filter, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createRITButton(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("RIT")), wxVERTICAL);

	m_ritCtrl = new wxToggleButton(window, RIT_BUTTON, _("On"), wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	sizer->Add(m_ritCtrl, 0, wxLEFT | wxRIGHT | wxBOTTOM, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createMuteButton(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Mute")), wxVERTICAL);

	m_mute = new wxToggleButton(window, MUTE_BUTTON, _("On"), wxDefaultPosition, wxSize(CONTROL_WIDTH, -1));

	sizer->Add(m_mute, 0, wxLEFT | wxRIGHT | wxBOTTOM, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createTransmitButton(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Transmit")), wxVERTICAL);

	m_transmit = new wxToggleButton(window, TX_BUTTON, _("On"), wxDefaultPosition, wxSize(TXBUTTON_WIDTH, TXBUTTON_HEIGHT));

	sizer->Add(m_transmit, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createRIT(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("RIT")), wxVERTICAL);

	m_rit = new CVolumeDial(window, RIT_KNOB, -5000, 5000, 0, this, wxDefaultPosition, wxSize(VOLSQL_WIDTH, VOLSQL_HEIGHT));

	sizer->Add(m_rit, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxPanel* CUWSDRFrame::createInfoBox(wxWindow* window)
{
	m_infoBox = new CInfoBox(window, INFO_DISPLAY, wxDefaultPosition, wxSize(INFO_WIDTH, INFO_HEIGHT));

	return m_infoBox;
}

wxPanel* CUWSDRFrame::createSMeter(wxWindow* window)
{
	m_sMeter = new CSMeter(window, SMETER, wxDefaultPosition, wxSize(SMETER_WIDTH, SMETER_HEIGHT));
	m_sMeter->setLevel(0);

	return m_sMeter;
}

wxSizer* CUWSDRFrame::createMicGain(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Mic Gain")), wxVERTICAL);

	m_micGain = new CVolumeDial(window, MIC_KNOB, 0, 1000, 0, this, wxDefaultPosition, wxSize(VOLSQL_WIDTH, VOLSQL_HEIGHT));

	sizer->Add(m_micGain, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createPowerControl(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Power")), wxVERTICAL);

	m_power = new CVolumeDial(window, POWER_KNOB, 0, 1000, 0, this, wxDefaultPosition, wxSize(VOLSQL_WIDTH, VOLSQL_HEIGHT));

	sizer->Add(m_power, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createAFGain(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("AF Gain")), wxVERTICAL);

	m_afGain = new CVolumeDial(window, VOLUME_KNOB, 0, 1000, 0, this, wxDefaultPosition, wxSize(VOLSQL_WIDTH, VOLSQL_HEIGHT));

	sizer->Add(m_afGain, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

wxSizer* CUWSDRFrame::createSquelch(wxWindow* window)
{
	wxStaticBoxSizer* sizer = new wxStaticBoxSizer(new wxStaticBox(window, -1, _("Squelch")), wxVERTICAL);

	m_squelch = new CVolumeDial(window, SQUELCH_KNOB, 0, 1000, 0, this, wxDefaultPosition, wxSize(VOLSQL_WIDTH, VOLSQL_HEIGHT));

	sizer->Add(m_squelch, 0, wxALL, BORDER_SIZE);
	sizer->SetSizeHints(window);

	return sizer;
}

void CUWSDRFrame::onMouseWheel(wxMouseEvent& event)
{
	int movement = event.GetWheelRotation();

	if (movement > 0)
		freqChange(2.0);
	else
		freqChange(-2.0);
}

void CUWSDRFrame::dialMoved(int id, int value)
{
	switch (id) {
		case FREQ_KNOB:
			freqChange(double(value) * m_stepSize);
			break;
		case RIT_KNOB:
			m_parameters->m_ritFreq = value;
			normaliseFreq();
			break;
		case MIC_KNOB:
			switch (m_parameters->m_mode) {
				case MODE_AM:
					m_parameters->m_amMicGain = value;
					m_dsp->setMicGain(value);
					break;
				case MODE_CWUW:
				case MODE_CWLW:
				case MODE_CWUN:
				case MODE_CWLN:
					m_dsp->setMicGain(75U);
					break;
				case MODE_FMN:
				case MODE_FMW:
					m_parameters->m_fmMicGain = value;
					m_dsp->setMicGain(value);
					break;
				case MODE_USB:
				case MODE_LSB:
					m_parameters->m_ssbMicGain = value;
					m_dsp->setMicGain(value);
					break;
				case MODE_DIGU:
				case MODE_DIGL:
					m_parameters->m_digMicGain = value;
					m_dsp->setMicGain(value);
					break;
			}
			break;
		case POWER_KNOB:
			switch (m_parameters->m_mode) {
				case MODE_AM:
					m_parameters->m_amPower = value;
					break;
				case MODE_CWUW:
				case MODE_CWLW:
				case MODE_CWUN:
				case MODE_CWLN:
					m_parameters->m_cwPower = value;
					break;
				case MODE_FMN:
				case MODE_FMW:
					m_parameters->m_fmPower = value;
					break;
				case MODE_USB:
				case MODE_LSB:
					m_parameters->m_ssbPower = value;
					break;
				case MODE_DIGU:
				case MODE_DIGL:
					m_parameters->m_digPower = value;
					break;
			}
			m_dsp->setPower(value);
			break;
		case VOLUME_KNOB:
			m_parameters->m_afGain = value;
			m_dsp->setAFGain(value);
			break;
		case SQUELCH_KNOB:
			m_parameters->m_squelch = value;
			m_dsp->setSquelch(value);
			break;
		case TUNING_HARDWARE: {
				wxCommandEvent event(TUNING_EVENT);
				event.SetInt(value);
				AddPendingEvent(event);
			}
			break;
	}
}

void CUWSDRFrame::freqChange(double value)
{
	CFrequency freq;

	if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		freq = m_parameters->m_vfoB + value;
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		freq = m_parameters->m_vfoA + value;
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		freq = m_parameters->m_vfoD + value;
	else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		freq = m_parameters->m_vfoC + value;

	else if (m_parameters->m_vfoChoice == VFO_A)
		freq = m_parameters->m_vfoA + value;
	else if (m_parameters->m_vfoChoice == VFO_B)
		freq = m_parameters->m_vfoB + value;
	else if (m_parameters->m_vfoChoice == VFO_C)
		freq = m_parameters->m_vfoC + value;
	else if (m_parameters->m_vfoChoice == VFO_D)
		freq = m_parameters->m_vfoD + value;

	if (m_txIntOn == 0U) {
		if (freq >= m_parameters->m_hardwareMaxFreq)
			freq = (freq - m_parameters->m_hardwareMaxFreq) + m_parameters->m_hardwareMinFreq;

		if (freq < m_parameters->m_hardwareMinFreq)
			freq = (freq - m_parameters->m_hardwareMinFreq) + m_parameters->m_hardwareMaxFreq;
	} else {
		if (!m_parameters->m_hardwareTXRange.inRange(freq)) {
			::wxBell();
			return;
		}
	}

	if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_parameters->m_vfoB = freq;
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_parameters->m_vfoA = freq;
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_parameters->m_vfoD = freq;
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_parameters->m_vfoC = freq;

	else if (m_parameters->m_vfoChoice == VFO_A)
		m_parameters->m_vfoA = freq;
	else if (m_parameters->m_vfoChoice == VFO_B)
		m_parameters->m_vfoB = freq;
	else if (m_parameters->m_vfoChoice == VFO_C)
		m_parameters->m_vfoC = freq;
	else if (m_parameters->m_vfoChoice == VFO_D)
		m_parameters->m_vfoD = freq;

	normaliseFreq();
}

void CUWSDRFrame::onMenuButton(wxCommandEvent& WXUNUSED(event))
{
	m_menu->Check(MENU_RECORD, m_record);

	PopupMenu(m_menu, 0, 30);
}

void CUWSDRFrame::onVFOButton(wxCommandEvent& event)
{
	if (m_txIntOn > 0U || m_txExtOn)
		return;

	switch (event.GetId()) {
		case VFO_AB_BUTTON:
			if (m_parameters->m_vfoChoice == VFO_A) {
				m_parameters->m_vfoChoice = VFO_B;
				m_infoBox->setVFO(VFO_B);
			} else {
				m_parameters->m_vfoChoice = VFO_A;
				m_infoBox->setVFO(VFO_A);
			}
			normaliseFreq();
			break;
		case VFO_CD_BUTTON:
			if (m_parameters->m_vfoChoice == VFO_C) {
				m_parameters->m_vfoChoice = VFO_D;
				m_infoBox->setVFO(VFO_D);
			} else {
				m_parameters->m_vfoChoice = VFO_C;
				m_infoBox->setVFO(VFO_C);
			}
			normaliseFreq();
			break;
		case VFO_SWAP_BUTTON:
			if (m_parameters->m_vfoChoice == VFO_A ||
				m_parameters->m_vfoChoice == VFO_B) {
				CFrequency temp = m_parameters->m_vfoA;
				m_parameters->m_vfoA = m_parameters->m_vfoB;
				m_parameters->m_vfoB = temp;
			} else {
				CFrequency temp = m_parameters->m_vfoC;
				m_parameters->m_vfoC = m_parameters->m_vfoD;
				m_parameters->m_vfoD = temp;
			}
			normaliseFreq();
			break;
		case VFO_SPLIT_BUTTON:
			if (m_parameters->m_vfoSplitShift == VFO_SPLIT)
				m_parameters->m_vfoSplitShift = VFO_NONE;
			else
				m_parameters->m_vfoSplitShift = VFO_SPLIT;
			m_infoBox->setSplitShift(m_parameters->m_vfoSplitShift);
			break;
		case VFO_SHIFT1_BUTTON:
			switch (m_parameters->m_vfoSplitShift) {
				case VFO_SHIFT1_PLUS:
					m_parameters->m_vfoSplitShift = VFO_SHIFT1_MINUS;
					break;
				case VFO_SHIFT1_MINUS:
					m_parameters->m_vfoSplitShift = VFO_NONE;
					break;
				default:
					m_parameters->m_vfoSplitShift = VFO_SHIFT1_PLUS;
					break;
			}
			m_infoBox->setSplitShift(m_parameters->m_vfoSplitShift);
			break;
		case VFO_SHIFT2_BUTTON:
			switch (m_parameters->m_vfoSplitShift) {
				case VFO_SHIFT2_PLUS:
					m_parameters->m_vfoSplitShift = VFO_SHIFT2_MINUS;
					break;
				case VFO_SHIFT2_MINUS:
					m_parameters->m_vfoSplitShift = VFO_NONE;
					break;
				default:
					m_parameters->m_vfoSplitShift = VFO_SHIFT2_PLUS;
					break;
			}
			m_infoBox->setSplitShift(m_parameters->m_vfoSplitShift);
			break;
	}
}

void CUWSDRFrame::onMHzButton(wxCommandEvent& event)
{
	if (m_txIntOn > 0U || m_txExtOn)
		return;

	switch (event.GetId()) {
		case FREQ_MHZ1_BUTTON:
			freqChange(-1000000.0);
			break;
		case FREQ_MHZ2_BUTTON:
			freqChange(1000000.0);
			break;
	}
}

void CUWSDRFrame::onModeChoice(wxCommandEvent& event)
{
	m_parameters->m_mode = UWSDRMODE(event.GetSelection());

	normaliseMode();

	normaliseFreq();
}

void CUWSDRFrame::onFilterChoice(wxCommandEvent& event)
{
	m_parameters->m_filter = FILTERWIDTH(event.GetSelection());

	normaliseMode();

	normaliseFreq();
}

void CUWSDRFrame::onRITButton(wxCommandEvent& event)
{
	m_parameters->m_ritOn = event.IsChecked();;

	m_infoBox->setRIT(m_parameters->m_ritOn);

	m_ritCtrl->SetLabel(m_parameters->m_ritOn ? _("Off") : _("On"));

	if (m_txIntOn > 0U || m_txExtOn)
		return;

	normaliseFreq();
}

void CUWSDRFrame::onMuteButton(wxCommandEvent& event)
{
	m_rxOn = event.IsChecked();

	m_mute->SetLabel(m_rxOn ? _("On") : _("Off"));

	m_sdr->enableRX(m_rxOn);
}

void CUWSDRFrame::onTXButton(wxCommandEvent& event)
{
	bool txOn = event.IsChecked();

	bool ret = normaliseIntTransmit(txOn);
	if (!ret) {
		m_transmit->SetValue(false);
		return;
	}

	m_transmit->SetLabel(txOn ? _("Off") : _("On"));
}

bool CUWSDRFrame::normaliseIntTransmit(bool txOn)
{
	if (m_txExtOn)
		return false;

	if (!txOn && m_txIntOn == 0U)
		return false;

	if (txOn && m_txIntOn > 0U) {
		m_txIntOn++;
		return true;
	}

	if (!txOn && m_txIntOn > 1U) {
		m_txIntOn--;
		return true;
	}

	// Sanity check on the transmit frequency
	if (m_txIntOn == 0U) {
		CFrequency freq;
		if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoA;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoB;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoC;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoD;

		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoB;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoA;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoD;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoC;

		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoA - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoA + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoA - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoA + m_parameters->m_freqShift2;

		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoB - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoB + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoB - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoB + m_parameters->m_freqShift2;

		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoC - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoC + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoC - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoC + m_parameters->m_freqShift2;

		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoD - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoD + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoD - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoD + m_parameters->m_freqShift2;

		if (!m_parameters->m_hardwareTXRange.inRange(freq)) {
			::wxBell();
			return false;
		}
	}

	if (txOn)
		m_txIntOn++;
	else
		m_txIntOn--;

	if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_infoBox->setVFO(VFO_B);
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_infoBox->setVFO(VFO_A);
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_infoBox->setVFO(VFO_D);
	else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn > 0U)
		m_infoBox->setVFO(VFO_C);
	else
		m_infoBox->setVFO(m_parameters->m_vfoChoice);

	m_infoBox->setTX(m_txIntOn > 0U);
	m_sMeter->setTX(m_txIntOn > 0U);

	return normaliseFreq();
}

bool CUWSDRFrame::normaliseExtTransmit(bool txOn)
{
	if (m_txIntOn > 0U)
		return false;

	// Sanity check on the transmit frequency
	if (!m_txExtOn) {
		CFrequency freq;
		if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoA;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoB;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoC;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_NONE)
			freq = m_parameters->m_vfoD;

		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoB;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoA;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoD;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT)
			freq = m_parameters->m_vfoC;

		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoA - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoA + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoA - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoA + m_parameters->m_freqShift2;

		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoB - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoB + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoB - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoB + m_parameters->m_freqShift2;

		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoC - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoC + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoC - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoC + m_parameters->m_freqShift2;

		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT1_MINUS)
			freq = m_parameters->m_vfoD - m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT1_PLUS)
			freq = m_parameters->m_vfoD + m_parameters->m_freqShift1;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT2_MINUS)
			freq = m_parameters->m_vfoD - m_parameters->m_freqShift2;
		else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SHIFT2_PLUS)
			freq = m_parameters->m_vfoD + m_parameters->m_freqShift2;

		if (!m_parameters->m_hardwareTXRange.inRange(freq))
			return false;
	}

	m_txExtOn = txOn;

	if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txExtOn)
		m_infoBox->setVFO(VFO_B);
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txExtOn)
		m_infoBox->setVFO(VFO_A);
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txExtOn)
		m_infoBox->setVFO(VFO_D);
	else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txExtOn)
		m_infoBox->setVFO(VFO_C);
	else
		m_infoBox->setVFO(m_parameters->m_vfoChoice);

	m_infoBox->setTX(m_txExtOn);
	m_sMeter->setTX(m_txExtOn);

	return normaliseFreq();
}

bool CUWSDRFrame::normaliseFreq()
{
	// We can be called too early ...
	if (m_parameters == NULL)
		return false;

	if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift != VFO_SPLIT)
		m_frequency = m_parameters->m_vfoA;
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift != VFO_SPLIT)
		m_frequency = m_parameters->m_vfoB;
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift != VFO_SPLIT)
		m_frequency = m_parameters->m_vfoC;
	else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift != VFO_SPLIT)
		m_frequency = m_parameters->m_vfoD;

	else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn == 0U && !m_txExtOn)
		m_frequency = m_parameters->m_vfoA;
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn == 0U && !m_txExtOn)
		m_frequency = m_parameters->m_vfoB;
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn == 0U && !m_txExtOn)
		m_frequency = m_parameters->m_vfoC;
	else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT && m_txIntOn == 0U && !m_txExtOn)
		m_frequency = m_parameters->m_vfoD;

	else if (m_parameters->m_vfoChoice == VFO_A && m_parameters->m_vfoSplitShift == VFO_SPLIT && (m_txIntOn > 0U || m_txExtOn))
		m_frequency = m_parameters->m_vfoB;
	else if (m_parameters->m_vfoChoice == VFO_B && m_parameters->m_vfoSplitShift == VFO_SPLIT && (m_txIntOn > 0U || m_txExtOn))
		m_frequency = m_parameters->m_vfoA;
	else if (m_parameters->m_vfoChoice == VFO_C && m_parameters->m_vfoSplitShift == VFO_SPLIT && (m_txIntOn > 0U || m_txExtOn))
		m_frequency = m_parameters->m_vfoD;
	else if (m_parameters->m_vfoChoice == VFO_D && m_parameters->m_vfoSplitShift == VFO_SPLIT && (m_txIntOn > 0U || m_txExtOn))
		m_frequency = m_parameters->m_vfoC;

	if (m_txIntOn > 0U || m_txExtOn) {
		switch (m_parameters->m_vfoSplitShift) {
			case VFO_SHIFT1_MINUS:
				m_frequency -= m_parameters->m_freqShift1;
				break;
			case VFO_SHIFT1_PLUS:
				m_frequency += m_parameters->m_freqShift1;
				break;
			case VFO_SHIFT2_MINUS:
				m_frequency -= m_parameters->m_freqShift2;
				break;
			case VFO_SHIFT2_PLUS:
				m_frequency += m_parameters->m_freqShift2;
				break;
			default:
				break;
		}
	}

	// Set the RIT
	if (m_parameters->m_ritOn && m_txIntOn == 0U && !m_txExtOn)
		m_frequency += double(m_parameters->m_ritFreq);

	// FM Wide is mapped to FM Narrow below 30 MHz
	if (m_parameters->m_mode == MODE_FMW && m_frequency.get() > wxInt64(0) && m_frequency.get() < wxInt64(30000000)) {
		m_parameters->m_mode = MODE_FMN;
		m_mode->SetSelection(MODE_FMN);
		normaliseMode();
	}

	CFrequency dispFreq = m_frequency;

	// Adjust the display ONLY frequency
	if (m_parameters->m_mode == MODE_CWUW || m_parameters->m_mode == MODE_CWUN)
		dispFreq -= CW_OFFSET;
	if (m_parameters->m_mode == MODE_CWLW || m_parameters->m_mode == MODE_CWLN)
		dispFreq += CW_OFFSET;

	if (m_parameters->m_freqOffset != 0.0)
		dispFreq += m_parameters->m_freqOffset;

	dispFreq = dispFreq + m_parameters->m_hardwareOffset;

	m_freqDisplay->setFrequency(dispFreq);

	// Subtract any offset frequency needed
	if (m_txIntOn > 0U || m_txExtOn)
		m_frequency -= m_dsp->getTXOffset();
	else
		m_frequency -= m_dsp->getRXOffset();

	// Take into account the frequency steps of the SDR ...
	unsigned int stepSize = m_parameters->m_hardwareStepSize;
	wxInt64 hz            = m_frequency.get();
	wxInt64 offset        = 0LL;

	offset = hz % stepSize;

	wxInt64 base = hz - offset;

	if (offset >= stepSize / 2U) {
		offset -= stepSize;
		base   += stepSize;
	}

	m_frequency.set(base);

	m_txInRange = m_parameters->m_hardwareTXRange.inRange(dispFreq);

	// If out of transmit frequency range then disable/remove transmit possibilities
	if (m_txInRange) {
		if (!m_swap->IsEnabled()) {		// Optimisation
			m_swap->Enable();
			m_split->Enable();
			m_shift1->Enable();
			m_shift2->Enable();
			m_mhzMinus->Enable();
			m_mhzPlus->Enable();
			m_ritCtrl->Enable();
			m_rit->Enable();
			m_transmit->Enable();
			// Only re-enable the mic gain when not in CW mode
			if (m_parameters->m_mode != MODE_CWUN && m_parameters->m_mode != MODE_CWLN &&
				m_parameters->m_mode != MODE_CWUW && m_parameters->m_mode != MODE_CWLW)
			m_micGain->Enable();
			m_power->Enable();
			m_sMeter->setTXMenu(true);
			m_menu->Enable(MENU_VOICE_KEYBOARD, true);
			m_menu->Enable(MENU_CW_KEYBOARD, true);
		}
	} else {
		if (m_swap->IsEnabled()) {		// Optimisation
			m_swap->Disable();
			m_split->Disable();
			m_shift1->Disable();
			m_shift2->Disable();
			m_mhzMinus->Disable();
			m_mhzPlus->Disable();
			m_ritCtrl->Disable();
			m_rit->Disable();
			m_transmit->Disable();
			m_micGain->Disable();
			m_power->Disable();
			m_sMeter->setTXMenu(false);
			m_menu->Enable(MENU_VOICE_KEYBOARD, false);
			m_menu->Enable(MENU_CW_KEYBOARD, false);
		}
	}

	// Finally go to TX or RX
	if (m_txIntOn > 0U) {
		if (m_txInRange) {
			m_sdr->setTXAndFreq(true, m_frequency);
			m_dsp->setTXAndFreq(TXSTATE_INT, offset);
			return true;
		}

		return false;
	} else if (m_txExtOn) {
		if (m_txInRange) {
			m_sdr->setTXAndFreq(true, m_frequency);
			m_dsp->setTXAndFreq(TXSTATE_EXT, offset);
			return true;
		}

		return false;
	} else {
		m_sdr->setTXAndFreq(false, m_frequency);
		m_dsp->setTXAndFreq(TXSTATE_RX, offset);
		return true;
	}
}

bool CUWSDRFrame::normaliseMode()
{
	// We can be called too early
	if (m_parameters == NULL)
		return false;

	m_dsp->setMode(m_parameters->m_mode);

	VFOSPEED    speed  = SPEED_MEDIUM;
	FILTERWIDTH filter = m_parameters->m_filter;

	unsigned int micGain = 500U;
	unsigned int power = 500U; 

	switch (m_parameters->m_mode) {
		case MODE_FMW:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterFMW;
			if (m_txInRange && !m_micGain->IsEnabled())
				m_micGain->Enable();
			m_dsp->setDeviation(m_parameters->m_deviationFMW);
			speed   = m_parameters->m_vfoSpeedFM;
			micGain = m_parameters->m_fmMicGain;
			power   = m_parameters->m_fmPower;
			break;
		case MODE_FMN:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterFMN;
			if (m_txInRange && !m_micGain->IsEnabled())
				m_micGain->Enable();
			m_dsp->setDeviation(m_parameters->m_deviationFMN);
			speed   = m_parameters->m_vfoSpeedFM;
			micGain = m_parameters->m_fmMicGain;
			power   = m_parameters->m_fmPower;
			break;
		case MODE_AM:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterAM;
			if (m_txInRange && !m_micGain->IsEnabled())
				m_micGain->Enable();
			m_dsp->setAGC(m_parameters->m_agcAM);
			speed   = m_parameters->m_vfoSpeedAM;
			micGain = m_parameters->m_amMicGain;
			power   = m_parameters->m_amPower;
			break;
		case MODE_USB:
		case MODE_LSB:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterSSB;
			if (m_txInRange && !m_micGain->IsEnabled())
				m_micGain->Enable();
			m_dsp->setAGC(m_parameters->m_agcSSB);
			speed   = m_parameters->m_vfoSpeedSSB;
			micGain = m_parameters->m_ssbMicGain;
			power   = m_parameters->m_ssbPower;
			break;
		case MODE_DIGU:
		case MODE_DIGL:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterDig;
			if (m_txInRange && !m_micGain->IsEnabled())
				m_micGain->Enable();
			m_dsp->setAGC(m_parameters->m_agcDig);
			speed   = m_parameters->m_vfoSpeedDig;
			micGain = m_parameters->m_digMicGain;
			power   = m_parameters->m_digPower;
			break;
		case MODE_CWUW:
		case MODE_CWLW:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterCWW;
			if (m_txInRange && m_micGain->IsEnabled())
				m_micGain->Disable();
			m_dsp->setAGC(m_parameters->m_agcCW);
			speed   = m_parameters->m_vfoSpeedCWW;
			micGain = 75U;
			power   = m_parameters->m_cwPower;
			break;
		case MODE_CWUN:
		case MODE_CWLN:
			if (filter == FILTER_AUTO)
				filter = m_parameters->m_filterCWN;
			if (m_txInRange && m_micGain->IsEnabled())
				m_micGain->Disable();
			m_dsp->setAGC(m_parameters->m_agcCW);
			speed   = m_parameters->m_vfoSpeedCWN;
			micGain = 75U;
			power   = m_parameters->m_cwPower;
			break;
	}

	m_dsp->setMicGain(micGain);
	m_micGain->setValue(micGain);

	m_dsp->setPower(power);
	m_power->setValue(power);

	m_dsp->setFilter(filter);
	m_spectrumDisplay->setFilter(filter, m_parameters->m_mode);

	switch (speed) {
		case SPEED_VERYFAST:
			m_stepSize = m_parameters->m_stepVeryFast;
			break;
		case SPEED_FAST:
			m_stepSize = m_parameters->m_stepFast;
			break;
		case SPEED_MEDIUM:
			m_stepSize = m_parameters->m_stepMedium;
			break;
		case SPEED_SLOW:
			m_stepSize = m_parameters->m_stepSlow;
			break;
		case SPEED_VERYSLOW:
			m_stepSize = m_parameters->m_stepVerySlow;
			break;
	}

	return true;
}

void CUWSDRFrame::onMenuSelection(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case MENU_PREFERENCES: {
				CUWSDRPreferences preferences(this, -1, m_parameters, m_dsp);
				int reply = preferences.ShowModal();
				if (reply == wxID_OK) {
					normaliseMode();

					m_dsp->setNB(m_parameters->m_nbOn);
					m_dsp->setNBValue(m_parameters->m_nbValue);
					m_dsp->setNB2(m_parameters->m_nb2On);
					m_dsp->setNB2Value(m_parameters->m_nb2Value);

					m_dsp->setSP(m_parameters->m_spOn);
					m_dsp->setSPValue(m_parameters->m_spValue);
					m_dsp->setCarrierLevel(m_parameters->m_carrierLevel);

					m_dsp->setALCValue(m_parameters->m_alcAttack, m_parameters->m_alcDecay, m_parameters->m_alcHang);

					m_dsp->setWeaver(m_parameters->m_weaver);

					m_dsp->setBinaural(m_parameters->m_binaural);
					m_dsp->setPan(m_parameters->m_pan);

					normaliseFreq();
				}

				// These may have been set in the preferences and then cancel pressed
				// so reset them to the saved values
				m_dsp->setRXIAndQ(m_parameters->m_rxIQphase, m_parameters->m_rxIQgain);
				m_dsp->setTXIAndQ(m_parameters->m_txIQphase, m_parameters->m_txIQgain);
				m_dsp->setRFGain(m_parameters->m_rfGain);

				int txEqVals[4U];
				txEqVals[0U] = m_parameters->m_txEqPreamp;
				txEqVals[1U] = m_parameters->m_txEqGain0;
				txEqVals[2U] = m_parameters->m_txEqGain1;
				txEqVals[3U] = m_parameters->m_txEqGain2;

				m_dsp->setEQ(m_parameters->m_txEqOn);
				m_dsp->setEQLevels(4U, txEqVals);
			}
			break;
		case MENU_KEYPAD: {
				if (m_txIntOn > 0U || m_txExtOn)
					return;

				CFrequency freq;
				if (m_parameters->m_vfoChoice == VFO_A)
					freq = m_parameters->m_vfoA;
				else if (m_parameters->m_vfoChoice == VFO_B)
					freq = m_parameters->m_vfoB;
				else if (m_parameters->m_vfoChoice == VFO_C)
					freq = m_parameters->m_vfoC;
				else if (m_parameters->m_vfoChoice == VFO_D)
					freq = m_parameters->m_vfoD;

				CFreqKeypad keypad(this, -1, freq, m_parameters->m_hardwareMinFreq, m_parameters->m_hardwareMaxFreq);
				int reply = keypad.ShowModal();

				if (reply == wxID_OK) {
					m_frequency = keypad.getFrequency();

					if (m_parameters->m_vfoChoice == VFO_A)
						m_parameters->m_vfoA = m_frequency;
					else if (m_parameters->m_vfoChoice == VFO_B)
						m_parameters->m_vfoB = m_frequency;
					else if (m_parameters->m_vfoChoice == VFO_C)
						m_parameters->m_vfoC = m_frequency;
					else if (m_parameters->m_vfoChoice == VFO_D)
						m_parameters->m_vfoD = m_frequency;

					normaliseFreq();
				}
			}
			break;
		case MENU_VOICE_KEYBOARD:
			m_voiceKeyboard->Show(true);
			break;
		case MENU_CW_KEYBOARD:
			m_cwKeyboard->Show(true);
			break;
		case MENU_RECORD:
			m_record = !m_record;

			if (m_record) {
				bool ret = m_dsp->setRecordOn(m_parameters->m_recordType);

				if (!ret) {
					m_record = false;
					::wxMessageBox(_("Cannot open the sound file for recording"), _("UWSDR Error"), wxICON_ERROR);
				}
			} else {
				m_dsp->setRecordOff();
			}
			break;
		case wxID_HELP:
			::wxGetApp().showHelp(1);
			break;
		case MENU_HARDWARE_INFO: {
				wxString stepSize;
				stepSize.Printf(wxT("%u"), m_parameters->m_hardwareStepSize);

				wxString sampleRate;
				sampleRate.Printf(wxT("%.0f"), m_parameters->m_hardwareSampleRate);

				wxString freqMult;
				freqMult.Printf(wxT("%u"), m_parameters->m_hardwareFreqMult);

				wxString txRanges;
				txRanges.Printf(wxT("%u"), m_parameters->m_hardwareTXRange.getCount());

				wxString type;
				switch (m_parameters->m_hardwareType) {
					case TYPE_DEMO:
						type = _("Demo");
						break;
					case TYPE_SI570RX:
						type = _("Si570 RX");
						break;
					case TYPE_SI570TXRX:
						type = _("Si570 TX/RX");
						break;
					case TYPE_HACKRF:
						type = _("HackRF");
						break;
				}

				wxString swapIQ = (m_parameters->m_hardwareSwapIQ) ? _("Yes") : _("No");

#if defined(__WXMSW__)
				::wxMessageBox(_("The hardware parameters are:\n\nName:\t\t") + m_parameters->m_hardwareName +
					_("\nMax. Frequency:\t") + m_parameters->m_hardwareMaxFreq.getString(3) +
					_(" MHz\nMin. Frequency:\t") + m_parameters->m_hardwareMinFreq.getString(3) +
					_(" MHz\nFrequency Mult:\t") + freqMult +
					_("\nStep Size:\t\t") + stepSize + _(" Hz\nSample Rate:\t") + sampleRate +
					_(" samples/sec\nType:\t\t") + type + _("\nTransmit Ranges:\t") + txRanges +
					_("\nSwap I/Q:\t\t") + swapIQ,
					_("SDR Hardware Information"),
					wxICON_INFORMATION);
#else
				::wxMessageBox(_("The hardware parameters are:\n\nName:\t\t\t\t") + m_parameters->m_hardwareName +
					_("\nMax. Frequency:\t") + m_parameters->m_hardwareMaxFreq.getString(3) +
					_(" MHz\nMin. Frequency:\t") + m_parameters->m_hardwareMinFreq.getString(3) +
					_(" MHz\nFrequency Mult:\t") + freqMult +
					_("\nStep Size:\t\t\t") + stepSize + _(" Hz\nSample Rate:\t\t") + sampleRate +
					_(" samples/sec\nType:\t\t\t\t") + type + _("\nTransmit Ranges:\t\t") + txRanges +
					_("\nSwap I/Q:\t\t\t") + swapIQ,
					_("SDR Hardware Information"),
					wxICON_INFORMATION);
#endif
			}
			break;
		case wxID_ABOUT: {
			// The wxABoutBox under Windows looks horrible, so use a home made one
			wxDateTime dateTime(REL_DATE_DAY, REL_DATE_MONTH, REL_DATE_YEAR);
			if (!dateTime.IsValid())
				return;

			wxString dateText = dateTime.Format(_("%d %B %Y"));

#if defined(__WXMSW__)
			::wxMessageBox(VERSION + wxT(" - ") + dateText +
				_("\n\nA Software Defined Radio GUI\n\nJonathan Naylor, G4KLX\nBob McGwier, N4HY\nFrank Brickle, AB2KT"),
				_("About UWSDR"),
				wxICON_INFORMATION);
#else
				wxAboutDialogInfo info;

				info.SetName(wxT("UWSDR"));
				info.SetVersion(VERSION + wxT(" - ") + dateText);
				info.SetDescription(_("A Software Defined Radio GUI"));

				info.AddDeveloper(wxT("Jonathan Naylor, G4KLX"));
				info.AddDeveloper(wxT("Bob McGwier, N4HY"));
				info.AddDeveloper(wxT("Frank Brickle, AB2KT"));

				::wxAboutBox(info);
#endif
			}
			break;
 		case wxID_EXIT:
			if (m_txIntOn > 0U || m_txExtOn)
				return;
			Close(false);
			break;
	}
}

void CUWSDRFrame::onTimer(wxTimerEvent& WXUNUSED(event))
{
	m_parameters->m_spectrumType  = m_spectrumDisplay->getType();
	m_parameters->m_spectrumSpeed = m_spectrumDisplay->getSpeed();
	m_parameters->m_spectrumDB    = m_spectrumDisplay->getDB();

	if (m_txIntOn > 0U || m_txExtOn) {
		METERPOS meter = m_sMeter->getTXMeter();
		m_parameters->m_txMeter = meter;

		float val = m_dsp->getMeter(meter);

		if (val != -200.0F) {
			m_sMeter->setLevel(val);

			float offset = 0.0F;
			if (m_parameters->m_weaver)
				offset = m_dsp->getTXOffset();

			switch (m_parameters->m_spectrumType) {
				case SPECTRUM_PHASE:
					m_dsp->getPhase(m_spectrum);
					break;
				case SPECTRUM_AUDIO:
					m_dsp->getScope(m_spectrum);
					break;
				default:
					m_dsp->getSpectrum(m_spectrum);
					break;
			}

			m_spectrumDisplay->showSpectrum(m_spectrum, 0.0F, offset);
			m_spectrumDisplay->getFreqPick();
		}
	} else {
		METERPOS meter = m_sMeter->getRXMeter();
		m_parameters->m_rxMeter = meter;

		float val = m_dsp->getMeter(meter);

		if (val != -200.0F) {
			m_sMeter->setLevel(val);

			float offset = 0.0F;
			if (m_parameters->m_weaver)
				offset = m_dsp->getRXOffset();

			switch (m_parameters->m_spectrumType) {
				case SPECTRUM_PHASE:
					m_dsp->getPhase(m_spectrum);
					break;
				case SPECTRUM_AUDIO:
					m_dsp->getScope(m_spectrum);
					break;
				default:
					m_dsp->getSpectrum(m_spectrum);
					break;
			}

			m_spectrumDisplay->showSpectrum(m_spectrum, -35.0F, offset);

			float freq = m_spectrumDisplay->getFreqPick();
			if (freq != 0.0F) {
				if (m_parameters->m_hardwareSwapIQ)
					freqChange(-freq);
				else
					freqChange(freq);
			}
		}
	}
}

void CUWSDRFrame::onClose(wxCloseEvent& event)
{
	if (!event.CanVeto()) {
		Destroy();
		return;
	}

	if (m_txIntOn > 0U || m_txExtOn) {
		event.Veto();
		return;
	}

	int reply = ::wxMessageBox(_("Do you want to exit UWSDR"),
		_("Exit UWSDR"),
		wxOK | wxCANCEL | wxICON_QUESTION);

	if (reply == wxOK) {
		m_timer.Stop();
		if (!m_parameters->m_hardwareTXRange.isEmpty())
			m_sdr->enableTX(false);
		m_sdr->enableRX(false);

		::wxSleep(1);		// Give time to shut down

		m_sdr->close();
		m_dsp->close();
		if (m_tuning != NULL)
			m_tuning->close();

		// Grab the parameters from the CW keyboard
		m_parameters->m_cwSpeed   = m_cwKeyboard->getSpeed();
		m_parameters->m_cwLocal   = m_cwKeyboard->getLocal();
		m_parameters->m_cwRemote  = m_cwKeyboard->getRemote();
		m_parameters->m_cwLocator = m_cwKeyboard->getLocator();
		m_parameters->m_cwReport  = m_cwKeyboard->getReport();
		m_parameters->m_cwSerial  = m_cwKeyboard->getSerial();
		for (unsigned int i = 0; i < CWKEYBOARD_COUNT; i++)
			m_parameters->m_cwMessage[i] = m_cwKeyboard->getMessage(i);

		// Grab the parameters from the voice keyer
		m_parameters->m_voiceDir = m_voiceKeyboard->getDir();
		for (unsigned int j = 0; j < VOICEKEYER_COUNT; j++)
			m_parameters->m_voiceFile[j] = m_voiceKeyboard->getFile(j);

		Destroy();
	} else {
		event.Veto();
	}
}

CWERROR CUWSDRFrame::sendCW(unsigned int speed, const wxString& text, CWSTATUS state)
{
	// If we're not in CW mode, ignore
	if (m_parameters->m_mode != MODE_CWUW && m_parameters->m_mode != MODE_CWUN && m_parameters->m_mode != MODE_CWLW && m_parameters->m_mode != MODE_CWLN)
		return CW_ERROR_MODE;

	switch (state) {
		case CW_TX_ON:
			setTransmit(true);
			return CW_ERROR_NONE;
		case CW_TX_OFF:
			setTransmit(false);
			return CW_ERROR_NONE;
		default:
			return m_dsp->sendCW(speed, text, state);
	}
}

VOICEERROR CUWSDRFrame::sendAudio(const wxString& fileName, VOICESTATUS state)
{
	// If we're not in a voice mode, ignore
	if (m_parameters->m_mode != MODE_USB && m_parameters->m_mode != MODE_LSB && m_parameters->m_mode != MODE_FMN && m_parameters->m_mode != MODE_FMW)
		return VOICE_ERROR_MODE;

	switch (state) {
		case VOICE_TX_ON:
			setTransmit(true);
			return VOICE_ERROR_NONE;
		case VOICE_TX_OFF:
			setTransmit(false);
			return VOICE_ERROR_NONE;
		default:
			return m_dsp->sendAudio(fileName, state);
	}
}

void CUWSDRFrame::setTransmit(bool txOn)
{
	wxEventType type = (txOn) ? TRANSMIT_ON_EVENT : TRANSMIT_OFF_EVENT;

	wxCommandEvent event(type);
	AddPendingEvent(event);
}

void CUWSDRFrame::setKey(bool keyOn)
{
	wxEventType type = (keyOn) ? KEY_ON_EVENT : KEY_OFF_EVENT;

	wxCommandEvent event(type);
	AddPendingEvent(event);
}

void CUWSDRFrame::onTransmitOn(wxEvent& WXUNUSED(event))
{
	normaliseIntTransmit(true);
}

void CUWSDRFrame::onTransmitOff(wxEvent& WXUNUSED(event))
{
	normaliseIntTransmit(false);
}

void CUWSDRFrame::onKeyOn(wxEvent& WXUNUSED(event))
{
	m_dsp->setKey(true);
}

void CUWSDRFrame::onKeyOff(wxEvent& WXUNUSED(event))
{
	m_dsp->setKey(false);
}

void CUWSDRFrame::commandAck(const wxString& WXUNUSED(message))
{
}

void CUWSDRFrame::commandNak(const wxString& message)
{
	wxCommandEvent event(COMMAND_NAK_EVENT);
	event.SetString(message);

	AddPendingEvent(event);
}

void CUWSDRFrame::commandError(const wxString& message)
{
	wxCommandEvent event(COMMAND_ERROR_EVENT);
	event.SetString(message);

	AddPendingEvent(event);
}

void CUWSDRFrame::connectionLost()
{
	wxCommandEvent event(CONNECTION_LOST_EVENT);

	AddPendingEvent(event);
}

void CUWSDRFrame::onCommandNak(wxEvent& event1)
{
	wxCommandEvent& event2 = dynamic_cast<wxCommandEvent&>(event1);
	wxString message = event2.GetString();

	wxLogError(wxT("Received a NAK from the SDR: ") + message);
	::wxMessageBox(_("Received a NAK from the SDR\n") + message, _("UWSDR Error"), wxICON_ERROR);
}

void CUWSDRFrame::onCommandError(wxEvent& event1)
{
	wxCommandEvent& event2 = dynamic_cast<wxCommandEvent&>(event1);
	wxString message = event2.GetString();

	wxLogError(message);
	::wxMessageBox(message, _("UWSDR Error"), wxICON_ERROR);
}

void CUWSDRFrame::onConnectionLost(wxEvent& WXUNUSED(event))
{
	wxLogError(wxT("Connection to the SDR lost"));
	::wxMessageBox(_("Connection to the SDR lost"), _("UWSDR Error"), wxICON_ERROR);

	Close(true);
}

void CUWSDRFrame::onTune(wxEvent& event1)
{
	wxCommandEvent& event2 = dynamic_cast<wxCommandEvent&>(event1);
	int value = event2.GetInt();

	dialMoved(FREQ_KNOB, value);
}

bool CUWSDRFrame::setExtTransmit(bool txOn)
{
	return normaliseExtTransmit(txOn);
}

bool CUWSDRFrame::setExtFrequency(const CFrequency& freq)
{
	if (freq >= m_parameters->m_hardwareMaxFreq)
		return false;

	if (freq < m_parameters->m_hardwareMinFreq)
		return false;

	if (m_parameters->m_vfoChoice == VFO_A)
		m_parameters->m_vfoA = freq;
	else if (m_parameters->m_vfoChoice == VFO_B)
		m_parameters->m_vfoB = freq;
	else if (m_parameters->m_vfoChoice == VFO_C)
		m_parameters->m_vfoC = freq;
	else if (m_parameters->m_vfoChoice == VFO_D)
		m_parameters->m_vfoD = freq;

	return normaliseFreq();
}

bool CUWSDRFrame::setExtMode(UWSDRMODE mode)
{
	m_parameters->m_mode = mode;

	// We set the mode to CWUN always, convert to CWLN below 10 MHz
	if (mode == MODE_CWUN && m_frequency < 10000000ULL)
		m_parameters->m_mode = MODE_CWLN;

	bool ret = normaliseMode();
	if (!ret)
		return false;

	return normaliseFreq();
}
