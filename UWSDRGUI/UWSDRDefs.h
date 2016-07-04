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

#ifndef	UWSDRDefs_H
#define	UWSDRDefs_H

// For the GUI
const wxString APPNAME     = wxT("UWSDR");

const int BORDER_SIZE      = 5;

#if defined(__WXMSW__)
const int BUTTON_HEIGHT    = 23;

const int FREQDIAL_WIDTH   = 150;
const int FREQDIAL_HEIGHT  = 150;

const int VOLSQL_WIDTH     = 75;
const int VOLSQL_HEIGHT    = 75;

const int TXBUTTON_WIDTH   = 90;
const int TXBUTTON_HEIGHT  = 75;

const int SPECTRUM_WIDTH   = 480;
const int SPECTRUM_HEIGHT  = 100;

const int FREQPAD_WIDTH    = 60;
const int FREQPAD_HEIGHT   = 30;

const int INFO_WIDTH       = 80;
const int INFO_HEIGHT      = 70;

const int FREQUENCY_WIDTH  = 400;
const int FREQUENCY_HEIGHT = 70;

const int CONTROL_WIDTH    = 100;

const int SMETER_WIDTH     = 195;
const int SMETER_HEIGHT    = 70;

const int CWTEXT_WIDTH     = 650;
const int CWKEYB_WIDTH     = 750;
const int REAL_TIME_HEIGHT = 50;

const int VOICETEXT_WIDTH  = 200;
const int VOICEKEYB_WIDTH  = 350;

#elif defined(__WXMAC__)
const int BUTTON_HEIGHT    = 30;

const int FREQDIAL_WIDTH   = 200;
const int FREQDIAL_HEIGHT  = 200;

const int VOLSQL_WIDTH     = 90;
const int VOLSQL_HEIGHT    = 90;

const int TXBUTTON_WIDTH   = 90;
const int TXBUTTON_HEIGHT  = 90;

const int SPECTRUM_WIDTH   = 540;
const int SPECTRUM_HEIGHT  = 130;

const int FREQPAD_WIDTH    = 60;
const int FREQPAD_HEIGHT   = 30;

const int INFO_WIDTH       = 100;
const int INFO_HEIGHT      = 85;

const int FREQUENCY_WIDTH  = 440;
const int FREQUENCY_HEIGHT = 85;

const int CONTROL_WIDTH    = 100;

const int SMETER_WIDTH     = 245;
const int SMETER_HEIGHT    = 85;

const int CWTEXT_WIDTH     = 650;
const int CWKEYB_WIDTH     = 750;
const int REAL_TIME_HEIGHT = 50;

const int VOICETEXT_WIDTH  = 225;
const int VOICEKEYB_WIDTH  = 325;

#elif defined(__WXGTK__)
const int BUTTON_HEIGHT    = 28;

const int FREQDIAL_WIDTH   = 160;
const int FREQDIAL_HEIGHT  = 160;

const int VOLSQL_WIDTH     = 90;
const int VOLSQL_HEIGHT    = 90;

const int TXBUTTON_WIDTH   = 90;
const int TXBUTTON_HEIGHT  = 90;

const int SPECTRUM_WIDTH   = 500;
const int SPECTRUM_HEIGHT  = 112;

const int FREQPAD_WIDTH    = 70;
const int FREQPAD_HEIGHT   = 25;

const int INFO_WIDTH       = 90;
const int INFO_HEIGHT      = 78;

const int FREQUENCY_WIDTH  = 410;
const int FREQUENCY_HEIGHT = 78;

const int CONTROL_WIDTH    = 105;

const int SMETER_WIDTH     = 224;
const int SMETER_HEIGHT    = 78;

const int CWTEXT_WIDTH     = 700;
const int CWKEYB_WIDTH     = 810;
const int REAL_TIME_HEIGHT = 50;

const int VOICETEXT_WIDTH  = 225;
const int VOICEKEYB_WIDTH  = 390;

#else
#error "Unknown platform"
#endif

enum VFOCHOICE {
	VFO_A,
	VFO_B,
	VFO_C,
	VFO_D
};

enum VFOMODE {
	VFO_SPLIT,
	VFO_SHIFT1_PLUS,
	VFO_SHIFT1_MINUS,
	VFO_SHIFT2_PLUS,
	VFO_SHIFT2_MINUS,
	VFO_NONE
};

enum VFOSPEED {
	SPEED_VERYFAST,
	SPEED_FAST,
	SPEED_MEDIUM,
	SPEED_SLOW,
	SPEED_VERYSLOW
};

enum FILTERWIDTH {
	FILTER_20000,
	FILTER_15000,
	FILTER_10000,
	FILTER_6000,
	FILTER_4000,
	FILTER_2600,
	FILTER_2100,
	FILTER_1000,
	FILTER_500,
	FILTER_250,
	FILTER_100,
	FILTER_50,
	FILTER_25,
	FILTER_AUTO
};

enum FMDEVIATION {
	DEVIATION_6000,
	DEVIATION_5000,
	DEVIATION_3000,
	DEVIATION_2500,
	DEVIATION_2000
};

enum AGCSPEED {
	AGC_FAST,
	AGC_MEDIUM,
	AGC_SLOW,
	AGC_NONE
};

enum METERPOS {
	METER_I_INPUT,
	METER_Q_INPUT,
	METER_SIGNAL,
	METER_AVG_SIGNAL,
	METER_MICROPHONE,
	METER_POWER,
	METER_ALC,
	METER_COMPRESSED
};

enum SPECTRUMTYPE {
	SPECTRUM_PANADAPTER1,
	SPECTRUM_PANADAPTER2,
	SPECTRUM_WATERFALL,
	SPECTRUM_PHASE,
	SPECTRUM_AUDIO
};

enum SPECTRUMSPEED {
	SPECTRUM_200MS,
	SPECTRUM_400MS,
	SPECTRUM_600MS,
	SPECTRUM_800MS,
	SPECTRUM_1000MS
};

enum SPECTRUMRANGE {
	SPECTRUM_10DB,
	SPECTRUM_20DB,
	SPECTRUM_30DB,
	SPECTRUM_40DB,
	SPECTRUM_50DB,
	SPECTRUM_60DB
};

enum VOICEERROR {
	VOICE_ERROR_NONE,
	VOICE_ERROR_MODE,
	VOICE_ERROR_TX,
	VOICE_ERROR_FILE
};

enum VOICESTATUS {
	VOICE_SINGLE,
	VOICE_CONTINUOUS,
	VOICE_ABORT,
	VOICE_TX_ON,
	VOICE_TX_OFF
};

enum CWERROR {
	CW_ERROR_NONE,
	CW_ERROR_MODE,
	CW_ERROR_TX
};

enum CWSTATUS {
	CW_SEND_TEXT,
	CW_SEND_CHAR,
	CW_STOP,
	CW_TX_ON,
	CW_TX_OFF
};

enum RECORDTYPE {
	RECORD_MONO_AUDIO,
	RECORD_STEREO_AUDIO,
	RECORD_STEREO_IQ
};

enum TXSTATE {
	TXSTATE_RX,
	TXSTATE_INT,
	TXSTATE_EXT
};

const unsigned int CWKEYBOARD_COUNT = 6U;

const unsigned int VOICEKEYER_COUNT = 20U;

const float CW_OFFSET            = 800.0F;

const double FREQ_VERY_FAST_STEP = 500.0;
const double FREQ_FAST_STEP      = 300.0;
const double FREQ_MEDIUM_STEP    = 100.0;
const double FREQ_SLOW_STEP      = 10.0;
const double FREQ_VERY_SLOW_STEP = 3.0;

const int SPECTRUM_SIZE = 4096;

const int      KEYER_SPEED  = 12;
const wxString KEYER_REPORT = wxT("599");
const wxString KEYER_SERIAL = wxT("001");

const wxString KEYER_MESSAGE[CWKEYBOARD_COUNT] = {
	wxT("CQ CQ CQ DE %M %M CQ CQ CQ DE %M %M CQ CQ CQ DE %M %M . K"),
	wxT("QRZ DE %M . K"),
	wxT("RR %R DE %M = TNX FER CALL = UR RST %S %S = MY LOC ES %Q %Q = SO HW? %R DE %M K"),
	wxT("RR %R DE %M = TNX FER QSO ES HPE CUAGN = 73 73 ES GUD DX = %R DE %M SK"),
	wxT("73 73 DE %M K"),
	wxT("%R DE %M %M . K")
};

#endif
