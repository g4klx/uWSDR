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

#include <wx/config.h>
#include <wx/cmdline.h>
#include <wx/fs_zip.h>
#include <wx/image.h>

#include "UWSDRApp.h"
#include "UWSDRDefs.h"
#include "SDRDescrFile.h"
#include "ConfigChooser.h"
#include "Log.h"

const wxChar* KEY_FILE_NAME          = wxT("/FileName");
const wxChar* KEY_VFO_A              = wxT("/VfoA");
const wxChar* KEY_VFO_B              = wxT("/VfoB");
const wxChar* KEY_VFO_C              = wxT("/VfoC");
const wxChar* KEY_VFO_D              = wxT("/VfoD");
const wxChar* KEY_FREQ_SHIFT1        = wxT("/FreqShift1");
const wxChar* KEY_FREQ_SHIFT2        = wxT("/FreqShift2");
const wxChar* KEY_FREQ_OFFSET        = wxT("/FreqOffset");
const wxChar* KEY_VFO_CHOICE         = wxT("/VfoChoice");
const wxChar* KEY_VFO_SPLIT_SHIFT    = wxT("/VfoSplitShift");
const wxChar* KEY_VFO_SPEED_FM       = wxT("/FM/VfoSpeed");
const wxChar* KEY_VFO_SPEED_AM       = wxT("/AM/VfoSpeed");
const wxChar* KEY_VFO_SPEED_SSB      = wxT("/SSB/VfoSpeed");
const wxChar* KEY_VFO_SPEED_CWW      = wxT("/CW/VfoSpeedW");
const wxChar* KEY_VFO_SPEED_CWN      = wxT("/CW/VfoSpeedN");
const wxChar* KEY_VFO_SPEED_DIG      = wxT("/Digital/VfoSpeed");
const wxChar* KEY_STEP_VERY_FAST     = wxT("/StepVeryFast");
const wxChar* KEY_STEP_FAST          = wxT("/StepFast");
const wxChar* KEY_STEP_MEDIUM        = wxT("/StepMedium");
const wxChar* KEY_STEP_SLOW          = wxT("/StepSlow");
const wxChar* KEY_STEP_VERY_SLOW     = wxT("/StepVerySlow");
const wxChar* KEY_MODE               = wxT("/Mode");
const wxChar* KEY_WEAVER             = wxT("/Weaver");
const wxChar* KEY_DEV_FMW            = wxT("/FM/DeviationW");
const wxChar* KEY_DEV_FMN            = wxT("/FM/DeviationN");
const wxChar* KEY_AGC_AM             = wxT("/AM/AGC");
const wxChar* KEY_AGC_SSB            = wxT("/SSB/AGC");
const wxChar* KEY_AGC_CW             = wxT("/CW/AGC");
const wxChar* KEY_AGC_DIG            = wxT("/Digital/AGC");
const wxChar* KEY_FILTER             = wxT("/Filter");
const wxChar* KEY_FILTER_FMW         = wxT("/FM/FilterW");
const wxChar* KEY_FILTER_FMN         = wxT("/FM/FilterN");
const wxChar* KEY_FILTER_AM          = wxT("/AM/Filter");
const wxChar* KEY_FILTER_SSB         = wxT("/SSB/Filter");
const wxChar* KEY_FILTER_CWW         = wxT("/CW/FilterW");
const wxChar* KEY_FILTER_CWN         = wxT("/CW/FilterN");
const wxChar* KEY_FILTER_DIG         = wxT("/Digital/Filter");
const wxChar* KEY_TX_IN_ENABLE       = wxT("/TXInEnable");
const wxChar* KEY_TX_IN_DEV          = wxT("/TXInDev");
const wxChar* KEY_TX_IN_PIN          = wxT("/TXInPin");
const wxChar* KEY_KEY_IN_ENABLE      = wxT("/KeyInEnable");
const wxChar* KEY_KEY_IN_DEV         = wxT("/KeyInDev");
const wxChar* KEY_KEY_IN_PIN         = wxT("/KeyInPin");
const wxChar* KEY_TX_OUT_DEV         = wxT("/TXOutDev");
const wxChar* KEY_TX_OUT_PIN         = wxT("/TXOutPin");
const wxChar* KEY_USER_AUDIO_TYPE    = wxT("/UserAudioType");
const wxChar* KEY_USER_AUDIO_IN_DEV  = wxT("/UserAudioInDev");
const wxChar* KEY_USER_AUDIO_OUT_DEV = wxT("/UserAudioOutDev");
const wxChar* KEY_SDR_AUDIO_TYPE     = wxT("/SDRAudioType");
const wxChar* KEY_SDR_AUDIO_IN_DEV   = wxT("/SDRAudioInDev");
const wxChar* KEY_SDR_AUDIO_OUT_DEV  = wxT("/SDRAudioOutDev");
const wxChar* KEY_RIT_CTRL           = wxT("/RitCtrl");
const wxChar* KEY_RIT_VALUE          = wxT("/RitFreq");
const wxChar* KEY_NB_CTRL            = wxT("/NoiseBlankerCtrl");
const wxChar* KEY_NB_VALUE           = wxT("/NoiseBlankerValue");
const wxChar* KEY_NB2_CTRL           = wxT("/NoiseBlanker2Ctrl");
const wxChar* KEY_NB2_VALUE          = wxT("/NoiseBlanker2Value");
const wxChar* KEY_SP_CTRL            = wxT("/SpeechProcCtrl");
const wxChar* KEY_SP_VALUE           = wxT("/SpeechprocValue");
const wxChar* KEY_CARRIER_LEVEL      = wxT("/CarrierLevel");
const wxChar* KEY_ALC_ATTACK         = wxT("/ALCAttackValue");
const wxChar* KEY_ALC_DECAY          = wxT("/ALCDecayValue");
const wxChar* KEY_ALC_HANG           = wxT("/ALCHangValue");
const wxChar* KEY_RX_METER           = wxT("/ReceiveMeter");
const wxChar* KEY_TX_METER           = wxT("/TransmitMeter");
const wxChar* KEY_SPECTRUM_TYPE      = wxT("/SpectrumType");
const wxChar* KEY_SPECTRUM_SPEED     = wxT("/SpectrumSpeed");
const wxChar* KEY_SPECTRUM_DB        = wxT("/SpectrumDB");
const wxChar* KEY_RX_IQ_PHASE        = wxT("/ReceiveIQPhase");
const wxChar* KEY_RX_IQ_GAIN         = wxT("/ReceiveIQGain");
const wxChar* KEY_TX_IQ_PHASE        = wxT("/TransmitIQPhase");
const wxChar* KEY_TX_IQ_GAIN         = wxT("/TransmitIQGain");
const wxChar* KEY_AF_GAIN            = wxT("/AFGain");
const wxChar* KEY_RF_GAIN            = wxT("/RFGain");
const wxChar* KEY_SQUELCH            = wxT("/Squelch");
const wxChar* KEY_AM_MIC_GAIN        = wxT("/AM/MicGain");
const wxChar* KEY_AM_POWER           = wxT("/AM/Power");
const wxChar* KEY_CW_POWER           = wxT("/CW/Power");
const wxChar* KEY_FM_MIC_GAIN        = wxT("/FM/MicGain");
const wxChar* KEY_FM_POWER           = wxT("/FM/Power");
const wxChar* KEY_SSB_MIC_GAIN       = wxT("/SSB/MicGain");
const wxChar* KEY_SSB_POWER          = wxT("/SSB/Power");
const wxChar* KEY_DIG_MIC_GAIN       = wxT("/Digital/MicGain");
const wxChar* KEY_DIG_POWER          = wxT("/Digital/Power");
const wxChar* KEY_BINAURAL           = wxT("/Binaural");
const wxChar* KEY_PAN                = wxT("/Pan");
const wxChar* KEY_RECORD_TYPE        = wxT("/RecordType");
const wxChar* KEY_TUNING_HW          = wxT("/TuningHW");
const wxChar* KEY_CW_SPEED           = wxT("/CW/Speed");
const wxChar* KEY_CW_LOCAL           = wxT("/CW/LocalCallsign");
const wxChar* KEY_CW_REMOTE          = wxT("/CW/RemoteCallsign");
const wxChar* KEY_CW_LOCATOR         = wxT("/CW/Locator");
const wxChar* KEY_CW_REPORT          = wxT("/CW/Report");
const wxChar* KEY_CW_SERIAL          = wxT("/CW/SerialNumber");
const wxChar* KEY_CW_MESSAGE         = wxT("/CW/Message");
const wxChar* KEY_VOICE_DIR          = wxT("/VoiceDir");
const wxChar* KEY_VOICE_FILE         = wxT("/VoiceFile");
const wxChar* KEY_EXT_NAME           = wxT("/ExternalName");
const wxChar* KEY_EXT_ADDRS          = wxT("/ExternalAddrs");
const wxChar* KEY_TXEQ_ENABLE        = wxT("/TXEQEnable");
const wxChar* KEY_TXEQ_PREAMP        = wxT("/TXEQPreamp");
const wxChar* KEY_TXEQ_GAIN1         = wxT("/TXEQGain1");
const wxChar* KEY_TXEQ_GAIN2         = wxT("/TXEQGain2");
const wxChar* KEY_TXEQ_GAIN3         = wxT("/TXEQGain3");

#if defined(__WXMSW__)
const wxChar* KEY_INST_PATH          = wxT("/InstPath");
#endif

const wxChar* SDR_NAME_PARAM         = wxT("SDR Name");


IMPLEMENT_APP(CUWSDRApp)

CUWSDRApp::CUWSDRApp() :
wxApp(),
m_frame(NULL),
m_parameters(NULL),
m_help(NULL)
{
	m_parameters = new CSDRParameters();
}

CUWSDRApp::~CUWSDRApp()
{
	delete m_parameters;
}

bool CUWSDRApp::OnInit()
{
	m_frame = new CUWSDRFrame;
	m_frame->Show();

	SetTopWindow(m_frame);

	if (!wxApp::OnInit())
		return false;

	wxFileName logFileName;
	logFileName.AssignHomeDir();
	logFileName.SetName(m_parameters->m_name);
	logFileName.SetExt(wxT("log"));

	wxLog* logger = new CLog(logFileName.GetFullPath());
	wxLog::SetActiveTarget(logger);

	::wxInitAllImageHandlers();

	wxFileSystem::AddHandler(new wxZipFSHandler);
	m_help = new wxHtmlHelpController();
	m_help->SetTitleFormat(_("UWSDR Help: %s"));

	wxFileName fileName;
	fileName.AssignDir(getHelpDir());
	fileName.SetFullName(wxT("UWSDR.zip"));
	m_help->AddBook(fileName);

	wxLogMessage(wxT("Starting ") + VERSION);
	wxLogMessage(wxT("Using configuration: ") + m_parameters->m_name);

	if (!readConfig()) {
		wxLogError(wxT("Cannot open the SDR configuration - ") + m_parameters->m_name);
		::wxMessageBox(_("Cannot open the SDR configuration - ") + m_parameters->m_name, _("UWSDR Error"), wxICON_ERROR);
		return false;
	}

	if (!readDescrFile()) {
		wxLogError(wxT("Cannot open the SDR description file - ") + m_parameters->m_fileName);
		::wxMessageBox(_("Cannot open the SDR description file - ") + m_parameters->m_fileName, _("UWSDR Error"), wxICON_ERROR);
		return false;
	}

	wxLogMessage(wxT("Using hardware configuration file: ") + m_parameters->m_fileName);

	// We cannot use Weaver on hardware with too large step sizes
	if (m_parameters->m_hardwareStepSize >= 100U        ||
	    m_parameters->m_hardwareType == TYPE_DEMO       ||
	    m_parameters->m_hardwareType == TYPE_SI570RX    ||
	    m_parameters->m_hardwareType == TYPE_SI570TXRX)
		m_parameters->m_weaver = false;

	if (m_parameters->m_vfoA > m_parameters->m_hardwareMaxFreq)
		m_parameters->m_vfoA = m_parameters->m_hardwareMaxFreq;

	if (m_parameters->m_vfoA < m_parameters->m_hardwareMinFreq)
		m_parameters->m_vfoA = m_parameters->m_hardwareMinFreq;

	if (m_parameters->m_vfoB > m_parameters->m_hardwareMaxFreq)
		m_parameters->m_vfoB = m_parameters->m_hardwareMaxFreq;

	if (m_parameters->m_vfoB < m_parameters->m_hardwareMinFreq)
		m_parameters->m_vfoB = m_parameters->m_hardwareMinFreq;

	if (m_parameters->m_vfoC > m_parameters->m_hardwareMaxFreq)
		m_parameters->m_vfoC = m_parameters->m_hardwareMaxFreq;

	if (m_parameters->m_vfoC < m_parameters->m_hardwareMinFreq)
		m_parameters->m_vfoC = m_parameters->m_hardwareMinFreq;

	if (m_parameters->m_vfoD > m_parameters->m_hardwareMaxFreq)
		m_parameters->m_vfoD = m_parameters->m_hardwareMaxFreq;

	if (m_parameters->m_vfoD < m_parameters->m_hardwareMinFreq)
		m_parameters->m_vfoD = m_parameters->m_hardwareMinFreq;

	wxString title = VERSION + wxT(" - ") + m_parameters->m_name;

	m_frame->SetTitle(title);

	m_frame->setParameters(m_parameters);

	return true;
}

void CUWSDRApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.AddParam(SDR_NAME_PARAM, wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL);

	wxApp::OnInitCmdLine(parser);
}

bool CUWSDRApp::OnCmdLineParsed(wxCmdLineParser& parser)
{
	if (!wxApp::OnCmdLineParsed(parser))
		return false;

	if (parser.GetParamCount() > 0U) {
		m_parameters->m_name = parser.GetParam(0);
		return true;
	}

	CConfigChooser configChooser(m_frame);
	int ret = configChooser.ShowModal();
	if (ret != wxID_OK)
		return false;

	m_parameters->m_name = configChooser.getName();

	return true;
}

int CUWSDRApp::OnExit()
{
	wxLogMessage(wxT("Ending UWSDR"));

	writeConfig();

	delete m_help;

	return 0;
}

bool CUWSDRApp::readDescrFile()
{
	CSDRDescrFile descrFile(m_parameters->m_fileName);

	if (!descrFile.isValid())
		return false;

	m_parameters->m_hardwareName              = descrFile.getName();
	m_parameters->m_hardwareType              = descrFile.getType();
	m_parameters->m_hardwareMaxFreq           = descrFile.getMaxFreq();
	m_parameters->m_hardwareMinFreq           = descrFile.getMinFreq();
	m_parameters->m_hardwareTXRange           = descrFile.getTXRanges();
	m_parameters->m_hardwareFreqMult          = descrFile.getFreqMult();
	m_parameters->m_hardwareOffset            = descrFile.getOffset();
	m_parameters->m_hardwareStepSize          = descrFile.getStepSize();
	m_parameters->m_hardwareSampleRate        = descrFile.getSampleRate();
	m_parameters->m_hardwareSwapIQ            = descrFile.getSwapIQ();
	m_parameters->m_hardwareReceiveGainOffset = descrFile.getReceiveGainOffset();

	return true;
}

bool CUWSDRApp::readConfig()
{
	wxString keyFileName        = wxT("/") + m_parameters->m_name + KEY_FILE_NAME;
	wxString keyVfoA            = wxT("/") + m_parameters->m_name + KEY_VFO_A;
	wxString keyVfoB            = wxT("/") + m_parameters->m_name + KEY_VFO_B;
	wxString keyVfoC            = wxT("/") + m_parameters->m_name + KEY_VFO_C;
	wxString keyVfoD            = wxT("/") + m_parameters->m_name + KEY_VFO_D;
	wxString keyFreqShift1      = wxT("/") + m_parameters->m_name + KEY_FREQ_SHIFT1;
	wxString keyFreqShift2      = wxT("/") + m_parameters->m_name + KEY_FREQ_SHIFT2;
	wxString keyFreqOffset      = wxT("/") + m_parameters->m_name + KEY_FREQ_OFFSET;
	wxString keyVfoChoice       = wxT("/") + m_parameters->m_name + KEY_VFO_CHOICE;
	wxString keyVfoSplitShift   = wxT("/") + m_parameters->m_name + KEY_VFO_SPLIT_SHIFT;
	wxString keyVfoSpeedFM      = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_FM;
	wxString keyVfoSpeedAM      = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_AM;
	wxString keyVfoSpeedSSB     = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_SSB;
	wxString keyVfoSpeedCWW     = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_CWW;
	wxString keyVfoSpeedCWN     = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_CWN;
	wxString keyVfoSpeedDig     = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_DIG;
	wxString keyDevFMW          = wxT("/") + m_parameters->m_name + KEY_DEV_FMW;
	wxString keyDevFMN          = wxT("/") + m_parameters->m_name + KEY_DEV_FMN;
	wxString keyAgcAM           = wxT("/") + m_parameters->m_name + KEY_AGC_AM;
	wxString keyAgcSSB          = wxT("/") + m_parameters->m_name + KEY_AGC_SSB;
	wxString keyAgcCW           = wxT("/") + m_parameters->m_name + KEY_AGC_CW;
	wxString keyAgcDig          = wxT("/") + m_parameters->m_name + KEY_AGC_DIG;
	wxString keyStepVeryFast    = wxT("/") + m_parameters->m_name + KEY_STEP_VERY_FAST;
	wxString keyStepFast        = wxT("/") + m_parameters->m_name + KEY_STEP_FAST;
	wxString keyStepMedium      = wxT("/") + m_parameters->m_name + KEY_STEP_MEDIUM;
	wxString keyStepSlow        = wxT("/") + m_parameters->m_name + KEY_STEP_SLOW;
	wxString keyStepVerySlow    = wxT("/") + m_parameters->m_name + KEY_STEP_VERY_SLOW;
	wxString keyMode            = wxT("/") + m_parameters->m_name + KEY_MODE;
	wxString keyWeaver          = wxT("/") + m_parameters->m_name + KEY_WEAVER;
	wxString keyFilter          = wxT("/") + m_parameters->m_name + KEY_FILTER;
	wxString keyFilterFMW       = wxT("/") + m_parameters->m_name + KEY_FILTER_FMW;
	wxString keyFilterFMN       = wxT("/") + m_parameters->m_name + KEY_FILTER_FMN;
	wxString keyFilterAM        = wxT("/") + m_parameters->m_name + KEY_FILTER_AM;
	wxString keyFilterSSB       = wxT("/") + m_parameters->m_name + KEY_FILTER_SSB;
	wxString keyFilterCWW       = wxT("/") + m_parameters->m_name + KEY_FILTER_CWW;
	wxString keyFilterCWN       = wxT("/") + m_parameters->m_name + KEY_FILTER_CWN;
	wxString keyFilterDig       = wxT("/") + m_parameters->m_name + KEY_FILTER_DIG;
	wxString keyTXInEnable      = wxT("/") + m_parameters->m_name + KEY_TX_IN_ENABLE;
	wxString keyTXInDev         = wxT("/") + m_parameters->m_name + KEY_TX_IN_DEV;
	wxString keyTXInPin         = wxT("/") + m_parameters->m_name + KEY_TX_IN_PIN;
	wxString keyKeyInEnable     = wxT("/") + m_parameters->m_name + KEY_KEY_IN_ENABLE;
	wxString keyKeyInDev        = wxT("/") + m_parameters->m_name + KEY_KEY_IN_DEV;
	wxString keyKeyInPin        = wxT("/") + m_parameters->m_name + KEY_KEY_IN_PIN;
	wxString keyTXOutDev        = wxT("/") + m_parameters->m_name + KEY_TX_OUT_DEV;
	wxString keyTXOutPin        = wxT("/") + m_parameters->m_name + KEY_TX_OUT_PIN;
	wxString keyUserAudioType   = wxT("/") + m_parameters->m_name + KEY_USER_AUDIO_TYPE;
	wxString keyUserAudioInDev  = wxT("/") + m_parameters->m_name + KEY_USER_AUDIO_IN_DEV;
	wxString keyUserAudioOutDev = wxT("/") + m_parameters->m_name + KEY_USER_AUDIO_OUT_DEV;
	wxString keySDRAudioType    = wxT("/") + m_parameters->m_name + KEY_SDR_AUDIO_TYPE;
	wxString keySDRAudioInDev   = wxT("/") + m_parameters->m_name + KEY_SDR_AUDIO_IN_DEV;
	wxString keySDRAudioOutDev  = wxT("/") + m_parameters->m_name + KEY_SDR_AUDIO_OUT_DEV;
	wxString keyRitCtrl         = wxT("/") + m_parameters->m_name + KEY_RIT_CTRL;
	wxString keyRitValue        = wxT("/") + m_parameters->m_name + KEY_RIT_VALUE;
	wxString keyNbCtrl          = wxT("/") + m_parameters->m_name + KEY_NB_CTRL;
	wxString keyNbValue         = wxT("/") + m_parameters->m_name + KEY_NB_VALUE;
	wxString keyNb2Ctrl         = wxT("/") + m_parameters->m_name + KEY_NB2_CTRL;
	wxString keyNb2Value        = wxT("/") + m_parameters->m_name + KEY_NB2_VALUE;
	wxString keySpCtrl          = wxT("/") + m_parameters->m_name + KEY_SP_CTRL;
	wxString keySpValue         = wxT("/") + m_parameters->m_name + KEY_SP_VALUE;
	wxString keyCarrierLevel    = wxT("/") + m_parameters->m_name + KEY_CARRIER_LEVEL;
	wxString keyAlcAttack       = wxT("/") + m_parameters->m_name + KEY_ALC_ATTACK;
	wxString keyAlcDecay        = wxT("/") + m_parameters->m_name + KEY_ALC_DECAY;
	wxString keyAlcHang         = wxT("/") + m_parameters->m_name + KEY_ALC_HANG;
	wxString keyRxMeter         = wxT("/") + m_parameters->m_name + KEY_RX_METER;
	wxString keyTxMeter         = wxT("/") + m_parameters->m_name + KEY_TX_METER;
	wxString keySpectrumType    = wxT("/") + m_parameters->m_name + KEY_SPECTRUM_TYPE;
	wxString keySpectrumSpeed   = wxT("/") + m_parameters->m_name + KEY_SPECTRUM_SPEED;
	wxString keySpectrumDB      = wxT("/") + m_parameters->m_name + KEY_SPECTRUM_DB;
	wxString keyRxIqPhase       = wxT("/") + m_parameters->m_name + KEY_RX_IQ_PHASE;
	wxString keyRxIqGain        = wxT("/") + m_parameters->m_name + KEY_RX_IQ_GAIN;
	wxString keyTxIqPhase       = wxT("/") + m_parameters->m_name + KEY_TX_IQ_PHASE;
	wxString keyTxIqGain        = wxT("/") + m_parameters->m_name + KEY_TX_IQ_GAIN;
	wxString keyAfGain          = wxT("/") + m_parameters->m_name + KEY_AF_GAIN;
	wxString keyRfGain          = wxT("/") + m_parameters->m_name + KEY_RF_GAIN;
	wxString keySquelch         = wxT("/") + m_parameters->m_name + KEY_SQUELCH;
	wxString keyAmMicGain       = wxT("/") + m_parameters->m_name + KEY_AM_MIC_GAIN;
	wxString keyAmPower         = wxT("/") + m_parameters->m_name + KEY_AM_POWER;
	wxString keyCwPower         = wxT("/") + m_parameters->m_name + KEY_CW_POWER;
	wxString keyFmMicGain       = wxT("/") + m_parameters->m_name + KEY_FM_MIC_GAIN;
	wxString keyFmPower         = wxT("/") + m_parameters->m_name + KEY_FM_POWER;
	wxString keySsbMicGain      = wxT("/") + m_parameters->m_name + KEY_SSB_MIC_GAIN;
	wxString keySsbPower        = wxT("/") + m_parameters->m_name + KEY_SSB_POWER;
	wxString keyDigMicGain      = wxT("/") + m_parameters->m_name + KEY_DIG_MIC_GAIN;
	wxString keyDigPower        = wxT("/") + m_parameters->m_name + KEY_DIG_POWER;
	wxString keyBinaural        = wxT("/") + m_parameters->m_name + KEY_BINAURAL;
	wxString keyPan             = wxT("/") + m_parameters->m_name + KEY_PAN;
	wxString keyRecordType      = wxT("/") + m_parameters->m_name + KEY_RECORD_TYPE;
	wxString keyTuningHW        = wxT("/") + m_parameters->m_name + KEY_TUNING_HW;
	wxString keyCwSpeed         = wxT("/") + m_parameters->m_name + KEY_CW_SPEED;
	wxString keyCwLocal         = wxT("/") + m_parameters->m_name + KEY_CW_LOCAL;
	wxString keyCwRemote        = wxT("/") + m_parameters->m_name + KEY_CW_REMOTE;
	wxString keyCwLocator       = wxT("/") + m_parameters->m_name + KEY_CW_LOCATOR;
	wxString keyCwReport        = wxT("/") + m_parameters->m_name + KEY_CW_REPORT;
	wxString keyCwSerial        = wxT("/") + m_parameters->m_name + KEY_CW_SERIAL;
	wxString keyVoiceDir        = wxT("/") + m_parameters->m_name + KEY_VOICE_DIR;
	wxString keyExtName         = wxT("/") + m_parameters->m_name + KEY_EXT_NAME;
	wxString keyExtAddrs        = wxT("/") + m_parameters->m_name + KEY_EXT_ADDRS;
	wxString keyTxEqEnable      = wxT("/") + m_parameters->m_name + KEY_TXEQ_ENABLE;
	wxString keyTxEqPreamp      = wxT("/") + m_parameters->m_name + KEY_TXEQ_PREAMP;
	wxString keyTxEqGain1       = wxT("/") + m_parameters->m_name + KEY_TXEQ_GAIN1;
	wxString keyTxEqGain2       = wxT("/") + m_parameters->m_name + KEY_TXEQ_GAIN2;
	wxString keyTxEqGain3       = wxT("/") + m_parameters->m_name + KEY_TXEQ_GAIN3;

	wxString keyCwMessage[CWKEYBOARD_COUNT];
	for (unsigned int i = 0; i < CWKEYBOARD_COUNT; i++) {
		wxString number;
		number.Printf(wxT("%d"), i);

		keyCwMessage[i] = wxT("/") + m_parameters->m_name + KEY_CW_MESSAGE;
		keyCwMessage[i].Append(number);
	}

	wxString keyVoiceFile[VOICEKEYER_COUNT];
	for (unsigned int j = 0; j < VOICEKEYER_COUNT; j++) {
		wxString number;
		number.Printf(wxT("%d"), j);

		keyVoiceFile[j] = wxT("/") + m_parameters->m_name + KEY_VOICE_FILE;
		keyVoiceFile[j].Append(number);
	}

	wxConfig* profile = new wxConfig(APPNAME);
	wxASSERT(profile != NULL);

	profile->SetRecordDefaults(true);

	if (!profile->Read(keyFileName, &m_parameters->m_fileName)) {
		delete profile;
		return false;
	}

	wxString freq;
	profile->Read(keyVfoA,             &freq, wxT("0.0"));
	m_parameters->m_vfoA.set(freq);

	profile->Read(keyVfoB,             &freq, wxT("0.0"));
	m_parameters->m_vfoB.set(freq);

	profile->Read(keyVfoC,             &freq, wxT("0.0"));
	m_parameters->m_vfoC.set(freq);

	profile->Read(keyVfoD,             &freq, wxT("0.0"));
	m_parameters->m_vfoD.set(freq);

	long num;
	profile->Read(keyFreqShift1,       &num, 0L);
	m_parameters->m_freqShift1 = num;

	profile->Read(keyFreqShift2,       &num, 0L);
	m_parameters->m_freqShift2 = num;

	profile->Read(keyFreqOffset,       &m_parameters->m_freqOffset, 0.0);

	profile->Read(keyVfoChoice,        &num, VFO_A);
	m_parameters->m_vfoChoice = VFOCHOICE(num);

	profile->Read(keyVfoSplitShift,    &num, VFO_NONE);
	m_parameters->m_vfoSplitShift = VFOMODE(num);

	profile->Read(keyVfoSpeedFM,       &num, SPEED_VERYFAST);
	m_parameters->m_vfoSpeedFM =  VFOSPEED(num);
	profile->Read(keyVfoSpeedAM,       &num, SPEED_MEDIUM);
	m_parameters->m_vfoSpeedAM =  VFOSPEED(num);
	profile->Read(keyVfoSpeedSSB,      &num, SPEED_SLOW);
	m_parameters->m_vfoSpeedSSB = VFOSPEED(num);
	profile->Read(keyVfoSpeedCWW,      &num, SPEED_SLOW);
	m_parameters->m_vfoSpeedCWW = VFOSPEED(num);
	profile->Read(keyVfoSpeedCWN,      &num, SPEED_VERYSLOW);
	m_parameters->m_vfoSpeedCWN = VFOSPEED(num);
	profile->Read(keyVfoSpeedDig,      &num, SPEED_SLOW);
	m_parameters->m_vfoSpeedDig = VFOSPEED(num);

	profile->Read(keyDevFMW,           &num, DEVIATION_5000);
	m_parameters->m_deviationFMW = FMDEVIATION(num);
	profile->Read(keyDevFMN,           &num, DEVIATION_2500);
	m_parameters->m_deviationFMN = FMDEVIATION(num);

	profile->Read(keyAgcAM,            &num, AGC_SLOW);
	m_parameters->m_agcAM  = AGCSPEED(num);
	profile->Read(keyAgcSSB,           &num, AGC_SLOW);
	m_parameters->m_agcSSB = AGCSPEED(num);
	profile->Read(keyAgcCW,            &num, AGC_FAST);
	m_parameters->m_agcCW  = AGCSPEED(num);
	profile->Read(keyAgcDig,           &num, AGC_SLOW);
	m_parameters->m_agcDig = AGCSPEED(num);

	profile->Read(keyStepVeryFast,     &m_parameters->m_stepVeryFast, FREQ_VERY_FAST_STEP);
	profile->Read(keyStepFast,         &m_parameters->m_stepFast,     FREQ_FAST_STEP);
	profile->Read(keyStepMedium,       &m_parameters->m_stepMedium,   FREQ_MEDIUM_STEP);
	profile->Read(keyStepSlow,         &m_parameters->m_stepSlow,     FREQ_SLOW_STEP);
	profile->Read(keyStepVerySlow,     &m_parameters->m_stepVerySlow, FREQ_VERY_SLOW_STEP);

	profile->Read(keyMode,             &num, MODE_USB);
	m_parameters->m_mode = UWSDRMODE(num);

	profile->Read(keyWeaver,           &m_parameters->m_weaver, true);

	profile->Read(keyFilter,           &num, FILTER_AUTO);
	m_parameters->m_filter =    FILTERWIDTH(num);
	profile->Read(keyFilterFMW,        &num, FILTER_20000);
	m_parameters->m_filterFMW = FILTERWIDTH(num);
	profile->Read(keyFilterFMN,        &num, FILTER_10000);
	m_parameters->m_filterFMN = FILTERWIDTH(num);
	profile->Read(keyFilterAM,         &num, FILTER_6000);
	m_parameters->m_filterAM =  FILTERWIDTH(num);
	profile->Read(keyFilterSSB,        &num, FILTER_2100);
	m_parameters->m_filterSSB = FILTERWIDTH(num);
	profile->Read(keyFilterCWW,        &num, FILTER_2100);
	m_parameters->m_filterCWW = FILTERWIDTH(num);
	profile->Read(keyFilterCWN,        &num, FILTER_500);
	m_parameters->m_filterCWN = FILTERWIDTH(num);
	profile->Read(keyFilterDig,        &num, FILTER_2100);
	m_parameters->m_filterDig = FILTERWIDTH(num);

	profile->Read(keyUserAudioType,    &num);
	m_parameters->m_userAudioType = SOUNDTYPE(num);
	profile->Read(keyUserAudioInDev,   &m_parameters->m_userAudioInDev);
	profile->Read(keyUserAudioOutDev,  &m_parameters->m_userAudioOutDev);

	profile->Read(keySDRAudioType,     &num);
	m_parameters->m_sdrAudioType = SOUNDTYPE(num);
	profile->Read(keySDRAudioInDev,    &m_parameters->m_sdrAudioInDev);
	profile->Read(keySDRAudioOutDev,   &m_parameters->m_sdrAudioOutDev);

	profile->Read(keyRitCtrl,          &m_parameters->m_ritOn,   false);
	profile->Read(keyRitValue,         &m_parameters->m_ritFreq, 0);

	profile->Read(keyNbCtrl,           &m_parameters->m_nbOn,    false);
	profile->Read(keyNbValue,          &num, 20);
	m_parameters->m_nbValue = num;

	profile->Read(keyNb2Ctrl,          &m_parameters->m_nb2On,   false);
	profile->Read(keyNb2Value,         &num, 15);
	m_parameters->m_nb2Value = num;

	profile->Read(keySpCtrl,           &m_parameters->m_spOn,    false);
	profile->Read(keySpValue,          &num, 3);
	m_parameters->m_spValue = num;

	profile->Read(keyCarrierLevel,     &num, 100);
	m_parameters->m_carrierLevel = num;

	profile->Read(keyAlcAttack,        &num, 2);
	m_parameters->m_alcAttack = num;

	profile->Read(keyAlcDecay,         &num, 10);
	m_parameters->m_alcDecay = num;

	profile->Read(keyAlcHang,          &num, 500);
	m_parameters->m_alcHang = num;

	profile->Read(keyRxMeter,          &num, METER_SIGNAL);
	m_parameters->m_rxMeter = METERPOS(num);
	profile->Read(keyTxMeter,          &num, METER_POWER);
	m_parameters->m_txMeter = METERPOS(num);

	profile->Read(keySpectrumType,     &num, SPECTRUM_PANADAPTER1);
	m_parameters->m_spectrumType =  SPECTRUMTYPE(num);
	profile->Read(keySpectrumSpeed,    &num, SPECTRUM_200MS);
	m_parameters->m_spectrumSpeed = SPECTRUMSPEED(num);
	profile->Read(keySpectrumDB,       &num, SPECTRUM_50DB);
	m_parameters->m_spectrumDB =    SPECTRUMRANGE(num);

	profile->Read(keyRxIqPhase,        &m_parameters->m_rxIQphase, 0);
	profile->Read(keyRxIqGain,         &m_parameters->m_rxIQgain,  0);
	profile->Read(keyTxIqPhase,        &m_parameters->m_txIQphase, 0);
	profile->Read(keyTxIqGain,         &m_parameters->m_txIQgain,  0);

	profile->Read(keyAfGain,           &num, 0);
	m_parameters->m_afGain = num;

	profile->Read(keyRfGain,           &num, 1000);
	m_parameters->m_rfGain = num;

	profile->Read(keySquelch,          &num, 0);
	m_parameters->m_squelch = num;

	profile->Read(keyAmMicGain,        &num, 0);
	m_parameters->m_amMicGain = num;

	profile->Read(keyAmPower,          &num, 0);
	m_parameters->m_amPower = num;

	profile->Read(keyCwPower,          &num, 0);
	m_parameters->m_cwPower = num;

	profile->Read(keyFmMicGain,        &num, 0);
	m_parameters->m_fmMicGain = num;

	profile->Read(keyFmPower,          &num, 0);
	m_parameters->m_fmPower = num;

	profile->Read(keySsbMicGain,       &num, 0);
	m_parameters->m_ssbMicGain = num;

	profile->Read(keySsbPower,         &num, 0);
	m_parameters->m_ssbPower = num;

	profile->Read(keyDigMicGain,       &num, 0);
	m_parameters->m_digMicGain = num;

	profile->Read(keyDigPower,         &num, 0);
	m_parameters->m_digPower = num;

	profile->Read(keyBinaural,         &m_parameters->m_binaural, false);
	profile->Read(keyPan,              &m_parameters->m_pan,      0);

	profile->Read(keyRecordType,       &num, RECORD_MONO_AUDIO);
	m_parameters->m_recordType = RECORDTYPE(num);

	profile->Read(keyTuningHW,         &num);
	m_parameters->m_tuning = TUNINGHW(num);

	profile->Read(keyCwSpeed,          &num, KEYER_SPEED);
	m_parameters->m_cwSpeed = num;

	profile->Read(keyCwLocal,          &m_parameters->m_cwLocal,   wxEmptyString);
	profile->Read(keyCwRemote,         &m_parameters->m_cwRemote,  wxEmptyString);
	profile->Read(keyCwLocator,        &m_parameters->m_cwLocator, wxEmptyString);
	profile->Read(keyCwReport,         &m_parameters->m_cwReport,  KEYER_REPORT);
	profile->Read(keyCwSerial,         &m_parameters->m_cwSerial,  KEYER_SERIAL);

	for (unsigned int n = 0; n < CWKEYBOARD_COUNT; n++)
		profile->Read(keyCwMessage[n], &m_parameters->m_cwMessage[n], KEYER_MESSAGE[n]);

	profile->Read(keyVoiceDir,         &m_parameters->m_voiceDir,  wxEmptyString);

	for (unsigned int m = 0; m < VOICEKEYER_COUNT; m++)
		profile->Read(keyVoiceFile[m], &m_parameters->m_voiceFile[m], wxEmptyString);

	profile->Read(keyExtName,          &m_parameters->m_externalName, wxEmptyString);
	profile->Read(keyExtAddrs,         &num, EXTERNALADDRS_HOST);
	m_parameters->m_externalAddrs = EXTERNALADDRS(num);

	profile->Read(keyTxEqEnable,       &m_parameters->m_txEqOn,     false);
	profile->Read(keyTxEqPreamp,       &m_parameters->m_txEqPreamp, 0);
	profile->Read(keyTxEqGain1,        &m_parameters->m_txEqGain0,  0);
	profile->Read(keyTxEqGain2,        &m_parameters->m_txEqGain1,  0);
	profile->Read(keyTxEqGain3,        &m_parameters->m_txEqGain2,  0);

	profile->Flush();

	delete profile;

	return true;
}

void CUWSDRApp::writeConfig()
{
	wxString keyVfoA          = wxT("/") + m_parameters->m_name + KEY_VFO_A;
	wxString keyVfoB          = wxT("/") + m_parameters->m_name + KEY_VFO_B;
	wxString keyVfoC          = wxT("/") + m_parameters->m_name + KEY_VFO_C;
	wxString keyVfoD          = wxT("/") + m_parameters->m_name + KEY_VFO_D;
	wxString keyFreqShift1    = wxT("/") + m_parameters->m_name + KEY_FREQ_SHIFT1;
	wxString keyFreqShift2    = wxT("/") + m_parameters->m_name + KEY_FREQ_SHIFT2;
	wxString keyFreqOffset    = wxT("/") + m_parameters->m_name + KEY_FREQ_OFFSET;
	wxString keyVfoChoice     = wxT("/") + m_parameters->m_name + KEY_VFO_CHOICE;
	wxString keyVfoSplitShift = wxT("/") + m_parameters->m_name + KEY_VFO_SPLIT_SHIFT;
	wxString keyVfoSpeedFM    = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_FM;
	wxString keyVfoSpeedAM    = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_AM;
	wxString keyVfoSpeedSSB   = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_SSB;
	wxString keyVfoSpeedCWW   = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_CWW;
	wxString keyVfoSpeedCWN   = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_CWN;
	wxString keyVfoSpeedDig   = wxT("/") + m_parameters->m_name + KEY_VFO_SPEED_DIG;
	wxString keyDevFMW        = wxT("/") + m_parameters->m_name + KEY_DEV_FMW;
	wxString keyDevFMN        = wxT("/") + m_parameters->m_name + KEY_DEV_FMN;
	wxString keyAgcAM         = wxT("/") + m_parameters->m_name + KEY_AGC_AM;
	wxString keyAgcSSB        = wxT("/") + m_parameters->m_name + KEY_AGC_SSB;
	wxString keyAgcCW         = wxT("/") + m_parameters->m_name + KEY_AGC_CW;
	wxString keyAgcDig        = wxT("/") + m_parameters->m_name + KEY_AGC_DIG;
	wxString keyStepVeryFast  = wxT("/") + m_parameters->m_name + KEY_STEP_VERY_FAST;
	wxString keyStepFast      = wxT("/") + m_parameters->m_name + KEY_STEP_FAST;
	wxString keyStepMedium    = wxT("/") + m_parameters->m_name + KEY_STEP_MEDIUM;
	wxString keyStepSlow      = wxT("/") + m_parameters->m_name + KEY_STEP_SLOW;
	wxString keyStepVerySlow  = wxT("/") + m_parameters->m_name + KEY_STEP_VERY_SLOW;
	wxString keyMode          = wxT("/") + m_parameters->m_name + KEY_MODE;
	wxString keyWeaver        = wxT("/") + m_parameters->m_name + KEY_WEAVER;
	wxString keyFilter        = wxT("/") + m_parameters->m_name + KEY_FILTER;
	wxString keyFilterFMW     = wxT("/") + m_parameters->m_name + KEY_FILTER_FMW;
	wxString keyFilterFMN     = wxT("/") + m_parameters->m_name + KEY_FILTER_FMN;
	wxString keyFilterAM      = wxT("/") + m_parameters->m_name + KEY_FILTER_AM;
	wxString keyFilterSSB     = wxT("/") + m_parameters->m_name + KEY_FILTER_SSB;
	wxString keyFilterCWW     = wxT("/") + m_parameters->m_name + KEY_FILTER_CWW;
	wxString keyFilterCWN     = wxT("/") + m_parameters->m_name + KEY_FILTER_CWN;
	wxString keyFilterDig     = wxT("/") + m_parameters->m_name + KEY_FILTER_DIG;
	wxString keyRitCtrl       = wxT("/") + m_parameters->m_name + KEY_RIT_CTRL;
	wxString keyRitValue      = wxT("/") + m_parameters->m_name + KEY_RIT_VALUE;
	wxString keyNbCtrl        = wxT("/") + m_parameters->m_name + KEY_NB_CTRL;
	wxString keyNbValue       = wxT("/") + m_parameters->m_name + KEY_NB_VALUE;
	wxString keyNb2Ctrl       = wxT("/") + m_parameters->m_name + KEY_NB2_CTRL;
	wxString keyNb2Value      = wxT("/") + m_parameters->m_name + KEY_NB2_VALUE;
	wxString keySpCtrl        = wxT("/") + m_parameters->m_name + KEY_SP_CTRL;
	wxString keySpValue       = wxT("/") + m_parameters->m_name + KEY_SP_VALUE;
	wxString keyCarrierLevel  = wxT("/") + m_parameters->m_name + KEY_CARRIER_LEVEL;
	wxString keyAlcAttack     = wxT("/") + m_parameters->m_name + KEY_ALC_ATTACK;
	wxString keyAlcDecay      = wxT("/") + m_parameters->m_name + KEY_ALC_DECAY;
	wxString keyAlcHang       = wxT("/") + m_parameters->m_name + KEY_ALC_HANG;
	wxString keyRxMeter       = wxT("/") + m_parameters->m_name + KEY_RX_METER;
	wxString keyTxMeter       = wxT("/") + m_parameters->m_name + KEY_TX_METER;
	wxString keySpectrumType  = wxT("/") + m_parameters->m_name + KEY_SPECTRUM_TYPE;
	wxString keySpectrumSpeed = wxT("/") + m_parameters->m_name + KEY_SPECTRUM_SPEED;
	wxString keySpectrumDB    = wxT("/") + m_parameters->m_name + KEY_SPECTRUM_DB;
	wxString keyRxIqPhase     = wxT("/") + m_parameters->m_name + KEY_RX_IQ_PHASE;
	wxString keyRxIqGain      = wxT("/") + m_parameters->m_name + KEY_RX_IQ_GAIN;
	wxString keyTxIqPhase     = wxT("/") + m_parameters->m_name + KEY_TX_IQ_PHASE;
	wxString keyTxIqGain      = wxT("/") + m_parameters->m_name + KEY_TX_IQ_GAIN;
	wxString keyAfGain        = wxT("/") + m_parameters->m_name + KEY_AF_GAIN;
	wxString keyRfGain        = wxT("/") + m_parameters->m_name + KEY_RF_GAIN;
	wxString keySquelch       = wxT("/") + m_parameters->m_name + KEY_SQUELCH;
	wxString keyAmMicGain     = wxT("/") + m_parameters->m_name + KEY_AM_MIC_GAIN;
	wxString keyAmPower       = wxT("/") + m_parameters->m_name + KEY_AM_POWER;
	wxString keyCwPower       = wxT("/") + m_parameters->m_name + KEY_CW_POWER;
	wxString keyFmMicGain     = wxT("/") + m_parameters->m_name + KEY_FM_MIC_GAIN;
	wxString keyFmPower       = wxT("/") + m_parameters->m_name + KEY_FM_POWER;
	wxString keySsbMicGain    = wxT("/") + m_parameters->m_name + KEY_SSB_MIC_GAIN;
	wxString keySsbPower      = wxT("/") + m_parameters->m_name + KEY_SSB_POWER;
	wxString keyDigMicGain    = wxT("/") + m_parameters->m_name + KEY_DIG_MIC_GAIN;
	wxString keyDigPower      = wxT("/") + m_parameters->m_name + KEY_DIG_POWER;
	wxString keyBinaural      = wxT("/") + m_parameters->m_name + KEY_BINAURAL;
	wxString keyPan           = wxT("/") + m_parameters->m_name + KEY_PAN;
	wxString keyRecordType    = wxT("/") + m_parameters->m_name + KEY_RECORD_TYPE;
	wxString keyCwSpeed       = wxT("/") + m_parameters->m_name + KEY_CW_SPEED;
	wxString keyCwLocal       = wxT("/") + m_parameters->m_name + KEY_CW_LOCAL;
	wxString keyCwRemote      = wxT("/") + m_parameters->m_name + KEY_CW_REMOTE;
	wxString keyCwLocator     = wxT("/") + m_parameters->m_name + KEY_CW_LOCATOR;
	wxString keyCwReport      = wxT("/") + m_parameters->m_name + KEY_CW_REPORT;
	wxString keyCwSerial      = wxT("/") + m_parameters->m_name + KEY_CW_SERIAL;
	wxString keyVoiceDir      = wxT("/") + m_parameters->m_name + KEY_VOICE_DIR;
	wxString keyExtName       = wxT("/") + m_parameters->m_name + KEY_EXT_NAME;
	wxString keyExtAddrs      = wxT("/") + m_parameters->m_name + KEY_EXT_ADDRS;
	wxString keyTxEqEnable    = wxT("/") + m_parameters->m_name + KEY_TXEQ_ENABLE;
	wxString keyTxEqPreamp    = wxT("/") + m_parameters->m_name + KEY_TXEQ_PREAMP;
	wxString keyTxEqGain1     = wxT("/") + m_parameters->m_name + KEY_TXEQ_GAIN1;
	wxString keyTxEqGain2     = wxT("/") + m_parameters->m_name + KEY_TXEQ_GAIN2;
	wxString keyTxEqGain3     = wxT("/") + m_parameters->m_name + KEY_TXEQ_GAIN3;

	wxString keyCwMessage[CWKEYBOARD_COUNT];
	for (unsigned int i = 0; i < CWKEYBOARD_COUNT; i++) {
		wxString number;
		number.Printf(wxT("%d"), i);

		keyCwMessage[i] = wxT("/") + m_parameters->m_name + KEY_CW_MESSAGE;
		keyCwMessage[i].Append(number);
	}

	wxString keyVoiceFile[VOICEKEYER_COUNT];
	for (unsigned int j = 0; j < VOICEKEYER_COUNT; j++) {
		wxString number;
		number.Printf(wxT("%d"), j);

		keyVoiceFile[j] = wxT("/") + m_parameters->m_name + KEY_VOICE_FILE;
		keyVoiceFile[j].Append(number);
	}

	wxConfig* profile = new wxConfig(APPNAME);
	wxASSERT(profile != NULL);

	profile->Write(keyVfoA,             m_parameters->m_vfoA.getString());
	profile->Write(keyVfoB,             m_parameters->m_vfoB.getString());
	profile->Write(keyVfoC,             m_parameters->m_vfoC.getString());
	profile->Write(keyVfoD,             m_parameters->m_vfoD.getString());
	profile->Write(keyFreqShift1,       int(m_parameters->m_freqShift1));
	profile->Write(keyFreqShift2,       int(m_parameters->m_freqShift2));
	profile->Write(keyFreqOffset,       m_parameters->m_freqOffset);
	profile->Write(keyVfoChoice,        m_parameters->m_vfoChoice);
	profile->Write(keyVfoSplitShift,    m_parameters->m_vfoSplitShift);
	profile->Write(keyVfoSpeedFM,       m_parameters->m_vfoSpeedFM);
	profile->Write(keyVfoSpeedAM,       m_parameters->m_vfoSpeedAM);
	profile->Write(keyVfoSpeedSSB,      m_parameters->m_vfoSpeedSSB);
	profile->Write(keyVfoSpeedCWW,      m_parameters->m_vfoSpeedCWW);
	profile->Write(keyVfoSpeedCWN,      m_parameters->m_vfoSpeedCWN);
	profile->Write(keyVfoSpeedDig,      m_parameters->m_vfoSpeedDig);
	profile->Write(keyDevFMW,           m_parameters->m_deviationFMW);
	profile->Write(keyDevFMN,           m_parameters->m_deviationFMN);
	profile->Write(keyAgcAM,            m_parameters->m_agcAM);
	profile->Write(keyAgcSSB,           m_parameters->m_agcSSB);
	profile->Write(keyAgcCW,            m_parameters->m_agcCW);
	profile->Write(keyAgcDig,           m_parameters->m_agcDig);
	profile->Write(keyStepVeryFast,     m_parameters->m_stepVeryFast);
	profile->Write(keyStepFast,         m_parameters->m_stepFast);
	profile->Write(keyStepMedium,       m_parameters->m_stepMedium);
	profile->Write(keyStepSlow,         m_parameters->m_stepSlow);
	profile->Write(keyStepVerySlow,     m_parameters->m_stepVerySlow);
	profile->Write(keyMode,             m_parameters->m_mode);
	profile->Write(keyWeaver,           m_parameters->m_weaver);
	profile->Write(keyFilter,           m_parameters->m_filter);
	profile->Write(keyFilterFMW,        m_parameters->m_filterFMW);
	profile->Write(keyFilterFMN,        m_parameters->m_filterFMN);
	profile->Write(keyFilterAM,         m_parameters->m_filterAM);
	profile->Write(keyFilterSSB,        m_parameters->m_filterSSB);
	profile->Write(keyFilterCWW,        m_parameters->m_filterCWW);
	profile->Write(keyFilterCWN,        m_parameters->m_filterCWN);
	profile->Write(keyFilterDig,        m_parameters->m_filterDig);
	profile->Write(keyRitCtrl,          m_parameters->m_ritOn);
	profile->Write(keyRitValue,         m_parameters->m_ritFreq);
	profile->Write(keyNbCtrl,           m_parameters->m_nbOn);
	profile->Write(keyNbValue,          int(m_parameters->m_nbValue));
	profile->Write(keyNb2Ctrl,          m_parameters->m_nb2On);
	profile->Write(keyNb2Value,         int(m_parameters->m_nb2Value));
	profile->Write(keySpCtrl,           m_parameters->m_spOn);
	profile->Write(keySpValue,          int(m_parameters->m_spValue));
	profile->Write(keyCarrierLevel,     int(m_parameters->m_carrierLevel));
	profile->Write(keyAlcAttack,        int(m_parameters->m_alcAttack));
	profile->Write(keyAlcDecay,         int(m_parameters->m_alcDecay));
	profile->Write(keyAlcHang,          int(m_parameters->m_alcHang));
	profile->Write(keyRxMeter,          m_parameters->m_rxMeter);
	profile->Write(keyTxMeter,          m_parameters->m_txMeter);
	profile->Write(keySpectrumType,     m_parameters->m_spectrumType);
	profile->Write(keySpectrumSpeed,    m_parameters->m_spectrumSpeed);
	profile->Write(keySpectrumDB,       m_parameters->m_spectrumDB);
	profile->Write(keyRxIqPhase,        m_parameters->m_rxIQphase);
	profile->Write(keyRxIqGain,         m_parameters->m_rxIQgain);
	profile->Write(keyTxIqPhase,        m_parameters->m_txIQphase);
	profile->Write(keyTxIqGain,         m_parameters->m_txIQgain);
	profile->Write(keyAfGain,           int(m_parameters->m_afGain));
	profile->Write(keyRfGain,           int(m_parameters->m_rfGain));
	profile->Write(keySquelch,          int(m_parameters->m_squelch));
	profile->Write(keyAmMicGain,        int(m_parameters->m_amMicGain));
	profile->Write(keyAmPower,          int(m_parameters->m_amPower));
	profile->Write(keyCwPower,          int(m_parameters->m_cwPower));
	profile->Write(keyFmMicGain,        int(m_parameters->m_fmMicGain));
	profile->Write(keyFmPower,          int(m_parameters->m_fmPower));
	profile->Write(keySsbMicGain,       int(m_parameters->m_ssbMicGain));
	profile->Write(keySsbPower,         int(m_parameters->m_ssbPower));
	profile->Write(keyDigMicGain,       int(m_parameters->m_digMicGain));
	profile->Write(keyDigPower,         int(m_parameters->m_digPower));
	profile->Write(keyBinaural,         m_parameters->m_binaural);
	profile->Write(keyPan,              m_parameters->m_pan);
	profile->Write(keyRecordType,       m_parameters->m_recordType);
	profile->Write(keyCwSpeed,          int(m_parameters->m_cwSpeed));
	profile->Write(keyCwLocal,          m_parameters->m_cwLocal);
	profile->Write(keyCwRemote,         m_parameters->m_cwRemote);
	profile->Write(keyCwLocator,        m_parameters->m_cwLocator);
	profile->Write(keyCwReport,         m_parameters->m_cwReport);
	profile->Write(keyCwSerial,         m_parameters->m_cwSerial);

	for (unsigned int n = 0; n < CWKEYBOARD_COUNT; n++)
		profile->Write(keyCwMessage[n], m_parameters->m_cwMessage[n]);

	profile->Write(keyVoiceDir,         m_parameters->m_voiceDir);

	for (unsigned int m = 0; m < CWKEYBOARD_COUNT; m++)
		profile->Write(keyVoiceFile[m], m_parameters->m_voiceFile[m]);

	profile->Write(keyExtName,          m_parameters->m_externalName);
	profile->Write(keyExtAddrs,         int(m_parameters->m_externalAddrs));

	profile->Write(keyTxEqEnable,       m_parameters->m_txEqOn);
	profile->Write(keyTxEqPreamp,       m_parameters->m_txEqPreamp);
	profile->Write(keyTxEqGain1,        m_parameters->m_txEqGain0);
	profile->Write(keyTxEqGain2,        m_parameters->m_txEqGain1);
	profile->Write(keyTxEqGain3,        m_parameters->m_txEqGain2);

	profile->Flush();

	delete profile;
}

void CUWSDRApp::showHelp(int id)
{
	m_help->Display(id);
}

CWERROR CUWSDRApp::sendCW(unsigned int speed, const wxString& text, CWSTATUS state)
{
	return m_frame->sendCW(speed, text, state);
}

VOICEERROR CUWSDRApp::sendAudio(const wxString& fileName, VOICESTATUS state)
{
	return m_frame->sendAudio(fileName, state);
}

void CUWSDRApp::setTransmit(bool txOn)
{
	m_frame->setTransmit(txOn);
}

void CUWSDRApp::setKey(bool keyOn)
{
	m_frame->setKey(keyOn);
}

wxString CUWSDRApp::getHelpDir()
{
#if defined(__WXMSW__)
	wxConfig* config = new wxConfig(APPNAME);
	wxASSERT(config != NULL);

	wxString dir;
	bool ret = config->Read(KEY_INST_PATH, &dir);

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

#if defined(__WXDEBUG__)
void CUWSDRApp::OnAssertFailure(const wxChar* file, int line, const wxChar* func, const wxChar* cond, const wxChar* msg)
{
	wxLogFatalError(wxT("Assertion failed on line %d of %s in file %s: %s %s"), line, func, file, cond, msg);
}
#endif
