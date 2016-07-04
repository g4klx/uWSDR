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

#ifndef	SDRParameters_H
#define	SDRParameters_H

#include <wx/wx.h>

#include "Common.h"
#include "AudioDevInfo.h"
#include "UWSDRDefs.h"
#include "FrequencyRangeArray.h"
#include "Frequency.h"
#include "SDRDescrFile.h"

class CSDRParameters {

    public:
	CSDRParameters();
	~CSDRParameters();

	wxString             m_hardwareName;
	SDRTYPE	             m_hardwareType;
	CFrequency           m_hardwareMaxFreq;
	CFrequency           m_hardwareMinFreq;
	CFrequencyRangeArray m_hardwareTXRange;
	unsigned int         m_hardwareFreqMult;
	CFrequency           m_hardwareOffset;
	unsigned int         m_hardwareStepSize;
	float                m_hardwareSampleRate;
	bool                 m_hardwareSwapIQ;
	unsigned int         m_hardwareReceiveGainOffset;

	wxString      m_fileName;
	wxString      m_name;

	CFrequency    m_vfoA;
	CFrequency    m_vfoB;
	CFrequency    m_vfoC;
	CFrequency    m_vfoD;
	unsigned int  m_freqShift1;
	unsigned int  m_freqShift2;
	double        m_freqOffset;
	VFOCHOICE     m_vfoChoice;
	VFOMODE       m_vfoSplitShift;

	VFOSPEED      m_vfoSpeedFM;
	VFOSPEED      m_vfoSpeedAM;
	VFOSPEED      m_vfoSpeedSSB;
	VFOSPEED      m_vfoSpeedCWW;
	VFOSPEED      m_vfoSpeedCWN;
	VFOSPEED      m_vfoSpeedDig;

	double        m_stepVeryFast;
	double        m_stepFast;
	double        m_stepMedium;
	double        m_stepSlow;
	double        m_stepVerySlow;

	UWSDRMODE     m_mode;
	bool          m_weaver;

	FMDEVIATION   m_deviationFMW;
	FMDEVIATION   m_deviationFMN;
	AGCSPEED      m_agcAM;
	AGCSPEED      m_agcSSB;
	AGCSPEED      m_agcCW;
	AGCSPEED      m_agcDig;

	FILTERWIDTH   m_filter;
	FILTERWIDTH   m_filterFMW;
	FILTERWIDTH   m_filterFMN;
	FILTERWIDTH   m_filterAM;
	FILTERWIDTH   m_filterSSB;
	FILTERWIDTH   m_filterCWW;
	FILTERWIDTH   m_filterCWN;
	FILTERWIDTH   m_filterDig;

	SOUNDTYPE     m_userAudioType;
	int           m_userAudioInDev;
	int           m_userAudioOutDev;

	SOUNDTYPE     m_sdrAudioType;
	int           m_sdrAudioInDev;
	int           m_sdrAudioOutDev;

	bool          m_ritOn;
	int           m_ritFreq;

	bool          m_nbOn;
	unsigned int  m_nbValue;
	bool          m_nb2On;
	unsigned int  m_nb2Value;

	bool          m_spOn;
	unsigned int  m_spValue;
	unsigned int  m_carrierLevel;

	unsigned int  m_alcAttack;
	unsigned int  m_alcDecay;
	unsigned int  m_alcHang;

	METERPOS      m_rxMeter;
	METERPOS      m_txMeter;

	SPECTRUMTYPE  m_spectrumType;
	SPECTRUMSPEED m_spectrumSpeed;
	SPECTRUMRANGE m_spectrumDB;

	int           m_rxIQphase;
	int           m_rxIQgain;
	int           m_txIQphase;
	int           m_txIQgain;

	bool          m_txEqOn;
	int           m_txEqPreamp;
	int           m_txEqGain0;
	int           m_txEqGain1;
	int           m_txEqGain2;

	unsigned int  m_afGain;
	unsigned int  m_rfGain;
	unsigned int  m_squelch;

	unsigned int  m_amMicGain;
	unsigned int  m_amPower;

	unsigned int  m_cwPower;

	unsigned int  m_fmMicGain;
	unsigned int  m_fmPower;

	unsigned int  m_ssbMicGain;
	unsigned int  m_ssbPower;

	unsigned int  m_digMicGain;
	unsigned int  m_digPower;

	bool          m_binaural;
	int           m_pan;

	RECORDTYPE    m_recordType;

	TUNINGHW      m_tuning;

	unsigned int  m_cwSpeed;
	wxString      m_cwLocal;
	wxString      m_cwRemote;
	wxString      m_cwLocator;
	wxString      m_cwReport;
	wxString      m_cwSerial;
	wxString      m_cwMessage[CWKEYBOARD_COUNT];

	wxString      m_voiceDir;
	wxString      m_voiceFile[VOICEKEYER_COUNT];

	wxString      m_externalName;
	EXTERNALADDRS m_externalAddrs;
};

#endif
