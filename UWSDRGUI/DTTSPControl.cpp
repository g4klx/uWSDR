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

#include "DTTSPControl.h"


CDTTSPControl::CDTTSPControl() :
wxThread(),
m_dttsp(NULL),
m_sampleRate(0.0F),
m_receiveGainOffset(0.0F),
m_blockSize(0),
m_filter(FILTER_2100),
m_mode(MODE_USB),
m_weaver(true),
m_rxFreq(99999.9F),
m_txFreq(99999.9F),
m_transmit(false),
m_deviation(DEVIATION_5000),
m_agc(AGC_MEDIUM),
m_nb(false),
m_nbValue(0),
m_nb2(false),
m_nb2Value(0),
m_sp(false),
m_spValue(0),
m_eq(false),
m_eqLevels(NULL),
m_carrierLevel(100),
m_attack(0),
m_decay(0),
m_hang(0),
m_rxPhase(0),
m_rxGain(0),
m_txPhase(0),
m_txGain(0),
m_binaural(false),
m_pan(0),
m_rfGain(999999U),
m_micGain(999999U),
m_power(999999U),
m_squelch(999999U),
m_started(false)
{
	m_eqLevels = new int[4U];

	::memset(m_eqLevels, 0x00U, 4U * sizeof(int));
}

CDTTSPControl::~CDTTSPControl()
{
	delete[] m_eqLevels;
}

void CDTTSPControl::open(float sampleRate, unsigned int receiveGainOffset, unsigned int blockSize, bool swapIQ)
{
	m_sampleRate = sampleRate;
	m_blockSize  = blockSize;

	m_receiveGainOffset = ::pow(10.0F, float(receiveGainOffset) / 10.0F);

	m_dttsp = new CDttSP(sampleRate, blockSize, swapIQ);
	m_dttsp->releaseUpdate();
	m_dttsp->setDCBlockFlag(true);
	m_dttsp->setRXSquelchFlag(true);
	m_dttsp->setSpectrumWindowType(HANN_WINDOW);

	Create();

	SetPriority(WXTHREAD_MAX_PRIORITY);

	Run();
}

void* CDTTSPControl::Entry()
{
	m_started = true;

	m_dttsp->process();

	m_started = false;

	return (void*)0;
}


void CDTTSPControl::setFilter(FILTERWIDTH filter)
{
	m_filter = filter;

	normaliseFilter();
}

void CDTTSPControl::setMode(UWSDRMODE mode)
{
	if (mode == m_mode)
		return;

	switch (mode) {
		case MODE_AM:
			m_dttsp->setMode(AM);
			break;
		case MODE_CWLW:
		case MODE_CWLN:
			m_dttsp->setMode(CWL);
			break;
		case MODE_CWUW:
		case MODE_CWUN:
			m_dttsp->setMode(CWU);
			break;
		case MODE_DIGL:
			m_dttsp->setMode(DIGL);
			break;
		case MODE_DIGU:
			m_dttsp->setMode(DIGU);
			break;
		case MODE_FMN:
			m_dttsp->setMode(FMN);
			break;
		case MODE_FMW:
			m_dttsp->setMode(FMN);
			break;
		case MODE_LSB:
			m_dttsp->setMode(LSB);
			break;
		case MODE_USB:
			m_dttsp->setMode(USB);
			break;
	}

	m_mode = mode;

	normaliseFilter();
}

void CDTTSPControl::setWeaver(bool onOff)
{
	if (onOff == m_weaver)
		return;

	m_dttsp->setWeaver(onOff);

	m_weaver = onOff;
}

void CDTTSPControl::setTXAndFreq(bool transmit, float freq)
{
	if (transmit) {
		if (freq != m_txFreq)
			m_dttsp->setTXFrequency(freq);

		if (transmit != m_transmit)
			m_dttsp->setTRX(TX);

		m_txFreq   = freq;
		m_transmit = true;
	} else {
		if (freq != m_rxFreq)
			m_dttsp->setRXFrequency(freq);

		if (transmit != m_transmit)
			m_dttsp->setTRX(RX);

		m_rxFreq   = freq;
		m_transmit = false;
	}
}

void CDTTSPControl::setAGC(AGCSPEED agc)
{
	if (agc == m_agc)
		return;

	switch (agc) {
		case AGC_FAST:
			m_dttsp->setAGCMode(agcFAST);
			break;
		case AGC_MEDIUM:
			m_dttsp->setAGCMode(agcMED);
			break;
		case AGC_SLOW:
			m_dttsp->setAGCMode(agcSLOW);
			break;
		case AGC_NONE:
			m_dttsp->setAGCMode(agcOFF);
			break;
	}

	m_agc = agc;
}

void CDTTSPControl::setDeviation(FMDEVIATION dev)
{
	if (dev == m_deviation)
		return;

	switch (dev) {
		case DEVIATION_6000:
			m_dttsp->setDeviation(6000.0F);
			break;
		case DEVIATION_5000:
			m_dttsp->setDeviation(5000.0F);
			break;
		case DEVIATION_3000:
			m_dttsp->setDeviation(3000.0F);
			break;
		case DEVIATION_2500:
			m_dttsp->setDeviation(2500.0F);
			break;
		case DEVIATION_2000:
			m_dttsp->setDeviation(2000.0F);
			break;
	}

	m_deviation = dev;
}

void CDTTSPControl::setNB(bool onOff)
{
	if (onOff == m_nb)
		return;

	m_dttsp->setNBFlag(onOff);

	m_nb = onOff;
}

void CDTTSPControl::setNBValue(unsigned int value)
{
	if (value == m_nbValue)
		return;

	m_dttsp->setNBThreshold(float(value));

	m_nbValue = value;
}

void CDTTSPControl::setNB2(bool onOff)
{
	if (onOff == m_nb2)
		return;

	m_dttsp->setNBSDROMFlag(onOff);

	m_nb2 = onOff;
}

void CDTTSPControl::setNB2Value(unsigned int value)
{
	if (value == m_nb2Value)
		return;

	m_dttsp->setNBSDROMThreshold(float(value));

	m_nb2Value = value;
}

void CDTTSPControl::setSP(bool onOff)
{
	if (onOff == m_sp)
		return;

	m_dttsp->setCompressionFlag(onOff);

	m_sp = onOff;
}

void CDTTSPControl::setSPValue(unsigned int value)
{
	if (value == m_spValue)
		return;

	m_dttsp->setCompressionLevel(float(value));

	m_spValue = value;
}

void CDTTSPControl::setEQ(bool onOff)
{
	if (onOff == m_eq)
		return;

	m_dttsp->setEqualiserFlag(onOff);

	m_eq = onOff;
}

void CDTTSPControl::setEQLevels(unsigned int n, const int* levels)
{
	if (levels[0U] == m_eqLevels[0U] && levels[1U] == m_eqLevels[1U] && levels[2U] == m_eqLevels[2U] && levels[3U] == m_eqLevels[3U])
		return;

	m_dttsp->setEqualiserLevels(n, levels);

	m_eqLevels[0U] = levels[0U];
	m_eqLevels[1U] = levels[1U];
	m_eqLevels[2U] = levels[2U];
	m_eqLevels[3U] = levels[3U];
}

void CDTTSPControl::setCarrierLevel(unsigned int value)
{
	if (value == m_carrierLevel)
		return;

	m_dttsp->setCarrierLevel(float(value) / 100.0F);

	m_carrierLevel = value;
}

void CDTTSPControl::setALCValue(unsigned int attack, unsigned int decay, unsigned int hang)
{
	if (attack == m_attack && decay == m_decay && hang == m_hang)
		return;

	m_dttsp->setALCAttack(float(attack));
	m_dttsp->setALCDecay(float(decay));
	m_dttsp->setALCHangTime(float(hang));

	m_attack = attack;
	m_decay  = decay;
	m_hang   = hang;
}

void CDTTSPControl::setRXIAndQ(int phase, int gain)
{
	if (phase == m_rxPhase && gain == m_rxGain)
		return;

	m_dttsp->setRXCorrectIQ(phase, gain);

	m_rxPhase = phase;
	m_rxGain  = gain;
}

void CDTTSPControl::setTXIAndQ(int phase, int gain)
{
	if (phase == m_txPhase && gain == m_txGain)
		return;

	m_dttsp->setTXCorrectIQ(phase, gain);

	m_txPhase = phase;
	m_txGain  = gain;
}

void CDTTSPControl::setBinaural(bool onOff)
{
	if (onOff == m_binaural)
		return;

	m_dttsp->setBinauralFlag(onOff);

	m_binaural = onOff;
}

void CDTTSPControl::setPan(int value)
{
	if (value == m_pan)
		return;

	float val = float(value + 100) / 200.0F;

	m_dttsp->setRXPan(val);

	m_pan = value;
}

void CDTTSPControl::setRFGain(unsigned int value)
{
	if (value == m_rfGain)
		return;

	// Map 0 - 1000 to 0.0 - 1.0 plus receive gain offset
	float gain = float(value) / 1000.0F + m_receiveGainOffset;

	m_dttsp->setRFGain(gain);

	m_rfGain = value;
}

void CDTTSPControl::setMicGain(unsigned int value)
{
	if (value == m_micGain)
		return;

	// Map 0 - 1000 to 0.0 - 5.0
	float gain = float(value) / 200.0F;

	m_dttsp->setMicGain(gain);

	m_micGain = value;
}

void CDTTSPControl::setPower(unsigned int value)
{
	if (value == m_power)
		return;

	// Map 0 - 1000 to 0.0 - 1.0
	float power = float(value) / 1000.0F;

	m_dttsp->setPower(power);

	m_power = value;
}

void CDTTSPControl::setSquelch(unsigned int value)
{
	if (value == m_squelch)
		return;

	// Map 0 - 1000 to -200.0 - 0.0
	float sql = float(value) / 5.0F - 200.0F;

	m_dttsp->setRXSquelchThreshold(sql);

	m_squelch = value;
}

float CDTTSPControl::getTXOffset()
{
	return m_dttsp->getTXOffset();
}

float CDTTSPControl::getRXOffset()
{
	return m_dttsp->getRXOffset();
}

float CDTTSPControl::getMeter(METERPOS type)
{
	if (!m_started)
		return -200.0F;

	float val = -200.0F;

	switch (type) {
		case METER_I_INPUT:
			val = m_dttsp->getMeter(ADC_REAL);
			if (val != -200.0F)
				val += 55.0F;
			break;
		case METER_Q_INPUT:
			val = m_dttsp->getMeter(ADC_IMAG);
			if (val != -200.0F)
				val += 55.0F;
			break;
		case METER_SIGNAL:
			val = m_dttsp->getMeter(SIGNAL_STRENGTH);
			if (val != -200.0F)
				val += 45.0F;
			break;
		case METER_AVG_SIGNAL:
			val = m_dttsp->getMeter(AVG_SIGNAL_STRENGTH);
			if (val != -200.0F)
				val += 45.0F;
			break;
		case METER_MICROPHONE:
			val = m_dttsp->getMeter(MIC);
			break;
		case METER_POWER:
			val = m_dttsp->getMeter(PWR);
			break;
		case METER_COMPRESSED:
			val = m_dttsp->getMeter(COMP);
			break;
		case METER_ALC:
			val = m_dttsp->getMeter(ALC);
			break;
	}

	return val;
}

void CDTTSPControl::getSpectrum(float* spectrum)
{
	wxASSERT(spectrum != NULL);

	if (!m_started)
		return;

	m_dttsp->setSpectrumType(SPEC_IQ);
	m_dttsp->getSpectrum(spectrum);
}

void CDTTSPControl::getScope(float* spectrum)
{
	wxASSERT(spectrum != NULL);

	if (!m_started)
		return;

	m_dttsp->setSpectrumType(SPEC_AUDIO);
	m_dttsp->getScope(spectrum, SPECTRUM_SIZE);
}

void CDTTSPControl::getPhase(float* spectrum)
{
	wxASSERT(spectrum != NULL);

	if (!m_started)
		return;

	m_dttsp->setSpectrumType(SPEC_IQ);
	m_dttsp->getPhase(spectrum, SPECTRUM_SIZE);
}

void CDTTSPControl::dataIO(const float* input, float* output, unsigned int nSamples)
{
	if (m_started)
		m_dttsp->audioEntry(input, output, nSamples);
}

void CDTTSPControl::close()
{
	delete m_dttsp;
}

void CDTTSPControl::normaliseFilter()
{
	double width = 0.0;
	switch (m_filter) {
		case FILTER_20000:
			width = 20000.0;
			break;
		case FILTER_15000:
			width = 15000.0;
			break;
		case FILTER_10000:
			width = 10000.0;
			break;
		case FILTER_6000:
			width = 6000.0;
			break;
		case FILTER_4000:
			width = 4000.0;
			break;
		case FILTER_2600:
			width = 2600.0;
			break;
		case FILTER_2100:
			width = 2100.0;
			break;
		case FILTER_1000:
			width = 1000.0;
			break;
		case FILTER_500:
			width = 500.0;
			break;
		case FILTER_250:
			width = 250.0;
			break;
		case FILTER_100:
			width = 100.0;
			break;
		case FILTER_50:
			width = 50.0;
			break;
		case FILTER_25:
			width = 25.0;
			break;
		case FILTER_AUTO:
			return;
	}

	double rxLow = 0.0, rxHigh = 0.0;
	double txLow = 0.0, txHigh = 0.0;
	switch (m_mode) {

		case MODE_FMW:
		case MODE_FMN:
		case MODE_AM:
			txHigh = rxHigh =  width / 2.0;
			txLow  = rxLow  = -width / 2.0;
			break;

		case MODE_USB:
		case MODE_LSB:
		case MODE_DIGL:
		case MODE_DIGU:
			switch (m_filter) {
				case FILTER_20000:
				case FILTER_15000:
				case FILTER_10000:
				case FILTER_6000:
				case FILTER_4000:
					txHigh = rxHigh = width + 100.0;
					txLow  = rxLow  = 100.0;
					break;
				case FILTER_2600:
				case FILTER_2100:
				case FILTER_1000:
					txHigh = rxHigh = width + 200.0;
					txLow  = rxLow  = 200.0;
					break;
				case FILTER_500:
					txHigh = rxHigh = 850.0;
					txLow  = rxLow  = 350.0;
					break;
				case FILTER_250:
					txHigh = rxHigh = 725.0;
					txLow  = rxLow  = 475.0;
					break;
				case FILTER_100:
					txHigh = rxHigh = 650.0;
					txLow  = rxLow  = 550.0;
					break;
				case FILTER_50:
					txHigh = rxHigh = 625.0;
					txLow  = rxLow  = 575.0;
					break;
				case FILTER_25:
					txHigh = rxHigh = 613.0;
					txLow  = rxLow  = 587.0;
					break;
				case FILTER_AUTO:
					return;
			}
			break;

		case MODE_CWUW:
		case MODE_CWUN:
		case MODE_CWLW:
		case MODE_CWLN:
			switch (m_filter) {
				case FILTER_20000:
				case FILTER_15000:
				case FILTER_10000:
				case FILTER_6000:
				case FILTER_4000:
				case FILTER_2600:
				case FILTER_2100:
					rxHigh = width + 100.0;
					rxLow  = 100.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_1000:
					rxHigh = CW_OFFSET + 500.0;
					rxLow  = CW_OFFSET - 500.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_500:
					rxHigh = CW_OFFSET + 250.0;
					rxLow  = CW_OFFSET - 250.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_250:
					rxHigh = CW_OFFSET + 125.0;
					rxLow  = CW_OFFSET - 125.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_100:
					rxHigh = CW_OFFSET + 50.0;
					rxLow  = CW_OFFSET - 50.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_50:
					rxHigh = CW_OFFSET + 25.0;
					rxLow  = CW_OFFSET - 25.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_25:
					rxHigh = CW_OFFSET + 13.0;
					rxLow  = CW_OFFSET - 12.0;
					txHigh = CW_OFFSET + 200.0;
					txLow  = CW_OFFSET - 200.0;
					break;
				case FILTER_AUTO:
					return;
			}
			break;
	}

	// Swap the filter values over
	if (m_mode == MODE_LSB || m_mode == MODE_CWLN || m_mode == MODE_CWLW || m_mode == MODE_DIGL) {
		double swap;

		swap   = rxLow;
		rxLow  = -rxHigh;
		rxHigh = -swap;

		swap   = txLow;
		txLow  = -txHigh;
		txHigh = -swap;
	}

	m_dttsp->setRXFilter(rxLow, rxHigh);
	m_dttsp->setTXFilter(txLow, txHigh);
}
