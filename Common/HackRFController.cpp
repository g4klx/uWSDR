/*
 *   Copyright (C) 2013 by Jonathan Naylor G4KLX
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
#include "HackRFController.h"

const unsigned int HACKRF_VID = 0x1D50;
const unsigned int HACKRF_PID = 0x604B;

const unsigned int REQUEST_SET_TRANSMIT  = 1U;
const unsigned int REQUEST_SET_FREQUENCY = 16U;

// TODO set sample rate, etc

CHackRFController::CHackRFController() :
m_context(NULL),
m_device(NULL),
m_callback(NULL),
m_frequency(),
m_txEnable(false),
m_tx(false)
{
	::libusb_init(&m_context);
}

CHackRFController::~CHackRFController()
{
	::libusb_exit(m_context);
}

bool CHackRFController::open()
{
	m_device = ::libusb_open_device_with_vid_pid(m_context, HACKRF_VID, HACKRF_PID);
	if (m_device == NULL) {
		wxLogError(wxT("Could not find the HackRF USB device"));
		return false;
	}

	int result = ::libusb_set_configuration(m_device, 1);
	if (result != 0) {
		wxLogError(wxT("Could not set the HackRF USB configuration"));
		::libusb_close(m_device);
		return false;
	}

	result = ::libusb_claim_interface(m_device, 0);
	if (result != 0) {
		wxLogError(wxT("Could not claim the HackRF USB interface"));
		::libusb_close(m_device);
		return false;
	}

	result = ::libusb_control_transfer(m_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, REQUEST_SET_TRANSMIT, 1U, 0U, NULL, 0UL, 0);
	if (result < 0) {
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), result);
		::libusb_close(m_device);
		return false;
	}

	wxLogInfo(wxT("Opened the HackRF device"));

	return true;
}

bool CHackRFController::setFrequency(const CFrequency& freq)
{
	/* Convert Freq Hz 64bits to Freq MHz (32bits) & Freq Hz (32bits) */
	wxUint32 lFreqMHz = wxUint32(freq.get() / wxUint64(1000000U));
	wxUint32 lFreqHz  = wxUint32(freq.get() % wxUint64(1000000U));

	struct {
		wxUint32 freqMHz;	/* From 30 to 6000MHz */
		wxUint32 freqHz;	/* From 0 to 999999Hz */
							/* Final Freq = freq_mhz+freq_hz */
	} setFreqParams;

	setFreqParams.freqMHz = wxUINT32_SWAP_ON_BE(lFreqMHz);
	setFreqParams.freqHz  = wxUINT32_SWAP_ON_BE(lFreqHz);

	wxUint8 length = sizeof(setFreqParams);
	int result = ::libusb_control_transfer(m_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, REQUEST_SET_FREQUENCY, 0, 0, (unsigned char*)&setFreqParams, length, 0);
	if (result < 0) {
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), result);
		return false;
	}

	return true;
}

bool CHackRFController::setTransmit(bool tx)
{
	uint16_t value = tx ? 2U : 1U;

	int n = ::libusb_control_transfer(m_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, REQUEST_SET_TRANSMIT, value, 0U, NULL, 0UL, 0);
	if (n < 0) {
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), n);
		return false;
	}

	return true;
}

void CHackRFController::close()
{
	int n = ::libusb_control_transfer(m_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, REQUEST_SET_TRANSMIT, 0U, 0U, NULL, 0UL, 0);
	if (n < 0)
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), n);

	::libusb_close(m_device);

	m_device = NULL;
}

void CHackRFController::setCallback(IControlInterface* callback)
{
	wxASSERT(callback != NULL);

	m_callback = callback;
}

void CHackRFController::enableTX(bool on)
{
	m_txEnable = on;
}

void CHackRFController::enableRX(bool WXUNUSED(on))
{
}

void CHackRFController::setTXAndFreq(bool transmit, const CFrequency& freq)
{
	wxASSERT(m_callback != NULL);

	if (transmit && !m_txEnable)
		return;

	if (freq != m_frequency) {
		bool ok = setFrequency(freq);
		if (!ok) {
			m_callback->connectionLost();
			return;
		}

		m_frequency = freq;
	}

	if (transmit != m_tx) {
		bool ok = setTransmit(transmit);
		if (!ok) {
			m_callback->connectionLost();
			return;
		}

		m_tx = transmit;
	}
}
