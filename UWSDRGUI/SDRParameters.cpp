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

#include "SDRParameters.h"

CSDRParameters::CSDRParameters() :
m_hardwareName(),
m_hardwareType(TYPE_DEMO),
m_hardwareMaxFreq(),
m_hardwareMinFreq(),
m_hardwareTXRange(),
m_hardwareFreqMult(1U),
m_hardwareOffset(),
m_hardwareStepSize(0U),
m_hardwareSampleRate(0.0F),
m_hardwareSwapIQ(false),
m_hardwareReceiveGainOffset(0U),
m_fileName(),
m_name(),
m_vfoA(),
m_vfoB(),
m_vfoC(),
m_vfoD(),
m_freqShift1(0U),
m_freqShift2(0U),
m_freqOffset(0.0),
m_vfoChoice(VFO_A),
m_vfoSplitShift(VFO_NONE),
m_vfoSpeedFM(SPEED_VERYFAST),
m_vfoSpeedAM(SPEED_MEDIUM),
m_vfoSpeedSSB(SPEED_SLOW),
m_vfoSpeedCWW(SPEED_SLOW),
m_vfoSpeedCWN(SPEED_VERYSLOW),
m_vfoSpeedDig(SPEED_SLOW),
m_stepVeryFast(0.0),
m_stepFast(0.0),
m_stepMedium(0.0),
m_stepSlow(0.0),
m_stepVerySlow(0.0),
m_mode(MODE_USB),
m_weaver(true),
m_deviationFMW(DEVIATION_5000),
m_deviationFMN(DEVIATION_2500),
m_agcAM(AGC_SLOW),
m_agcSSB(AGC_SLOW),
m_agcCW(AGC_FAST),
m_agcDig(AGC_SLOW),
m_filter(FILTER_AUTO),
m_filterFMW(FILTER_20000),
m_filterFMN(FILTER_10000),
m_filterAM(FILTER_6000),
m_filterSSB(FILTER_2100),
m_filterCWW(FILTER_2100),
m_filterCWN(FILTER_500),
m_filterDig(FILTER_2100),
m_userAudioType(),
m_userAudioInDev(-1),
m_userAudioOutDev(-1),
m_sdrAudioType(),
m_sdrAudioInDev(-1),
m_sdrAudioOutDev(-1),
m_ritOn(false),
m_ritFreq(0),
m_nbOn(false),
m_nbValue(0),
m_nb2On(false),
m_nb2Value(0),
m_spOn(false),
m_spValue(0),
m_carrierLevel(100),
m_alcAttack(0),
m_alcDecay(0),
m_alcHang(0),
m_rxMeter(METER_SIGNAL),
m_txMeter(METER_POWER),
m_spectrumType(SPECTRUM_PANADAPTER1),
m_spectrumSpeed(SPECTRUM_200MS),
m_spectrumDB(SPECTRUM_50DB),
m_rxIQphase(0),
m_rxIQgain(0),
m_txIQphase(0),
m_txIQgain(0),
m_txEqOn(false),
m_txEqPreamp(0),
m_txEqGain0(0),
m_txEqGain1(0),
m_txEqGain2(0),
m_afGain(0),
m_rfGain(1000),
m_squelch(0),
m_amMicGain(0),
m_amPower(0),
m_cwPower(0),
m_fmMicGain(0),
m_fmPower(0),
m_ssbMicGain(0),
m_ssbPower(0),
m_digMicGain(0),
m_digPower(0),
m_binaural(false),
m_pan(0),
m_recordType(RECORD_MONO_AUDIO),
m_tuning(TUNINGHW_NONE),
m_cwSpeed(0),
m_cwLocal(),
m_cwRemote(),
m_cwLocator(),
m_cwReport(),
m_cwSerial(),
m_cwMessage(),
m_voiceDir(),
m_voiceFile(),
m_externalName(),
m_externalAddrs(EXTERNALADDRS_HOST)
{
}

CSDRParameters::~CSDRParameters()
{
}
