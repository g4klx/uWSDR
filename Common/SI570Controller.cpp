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
#include "SI570Controller.h"

const unsigned int SI570_VID = 0x16C0;
const unsigned int SI570_PID = 0x05DC;

const unsigned int REQUEST_GET_VERSION   = 0x00U;
const unsigned int REQUEST_SET_FREQUENCY = 0x32U;
const unsigned int REQUEST_SET_TRANSMIT  = 0x50U;

#if defined(_WIN32)

CSI570Controller::CSI570Controller(unsigned int freqMult) :
m_freqMult(freqMult),
m_handle(NULL),
m_callback(NULL),
m_frequency(),
m_txEnable(false),
m_tx(false)
{
}

CSI570Controller::~CSI570Controller()
{
}

bool CSI570Controller::open()
{
	::usb_init();

	::usb_find_busses();
	::usb_find_devices();

	for (struct usb_bus* bus = ::usb_get_busses(); bus !=NULL; bus = bus->next) {
		for (struct usb_device* dev = bus->devices; dev != NULL; dev = dev->next) {
			if (dev->descriptor.idVendor == SI570_VID && dev->descriptor.idProduct == SI570_PID) {
				m_handle = ::usb_open(dev);
				if (m_handle == NULL) {
					wxString err(::usb_strerror(), wxConvLocal);
					wxLogError(wxT("Could not open the USB device: %s"), err.c_str());
					return false;
				}

				wxLogInfo(wxT("Found the Si570 USB device"));

				char version[2U];
				int n = ::usb_control_msg(m_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_GET_VERSION, 0xE00U, 0U, version, 2U, 500);
				if (n < 0) {
					wxString err(::usb_strerror(), wxConvLocal);
					wxLogError(wxT("Error from usb_control_msg: %s"), err.c_str());
					::usb_close(m_handle);
					return false;
				} else if (n == 2) {
					wxLogMessage(wxT("SI570Controller version: %d.%d"), version[1U], version[0U]);
					return true;
				} else {
					wxLogMessage(wxT("SI570Controller version: unknown"));
					return true;
				}
			}
		}
	}

	wxLogError(wxT("Could not find the Si570 USB device"));

	return false;
}

bool CSI570Controller::setFrequency(const CFrequency& freq)
{
	double dFrequency = double(freq.get()) / 1000000.0;

	wxUint32 frequency = wxUint32(dFrequency * (1UL << 21)) * m_freqMult;

	int n = ::usb_control_msg(m_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT, REQUEST_SET_FREQUENCY, 0U, 0U, (char*)&frequency, 4U, 500);
	if (n < 0) {
		wxString err(::usb_strerror(), wxConvLocal);
		wxLogError(wxT("Error from usb_control_msg: %s"), err.c_str());
		return false;
	}

	return true;
}

bool CSI570Controller::setTransmit(bool tx)
{
	wxUint16 value = tx ? 0x01U : 0x00U;

	char key;
	int n = ::usb_control_msg(m_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, REQUEST_SET_TRANSMIT, value, 0U, &key, 1U, 500);
	if (n < 0) {
		wxString err(::usb_strerror(), wxConvLocal);
		wxLogError(wxT("Error from usb_control_msg: %s"), err.c_str());
		return false;
	}

	return true;
}

void CSI570Controller::close()
{
	::usb_close(m_handle);
	m_handle = NULL;
}

#else

CSI570Controller::CSI570Controller(unsigned int freqMult) :
m_freqMult(freqMult),
m_context(NULL),
m_device(NULL),
m_callback(NULL),
m_frequency(),
m_txEnable(false),
m_tx(false)
{
	::libusb_init(&m_context);
}

CSI570Controller::~CSI570Controller()
{
	::libusb_exit(m_context);
}

bool CSI570Controller::open()
{
	m_device = ::libusb_open_device_with_vid_pid(m_context, SI570_VID, SI570_PID);
	if (m_device == NULL) {
		wxLogError(wxT("Could not find the Si570 USB device"));
		return false;
	}

	wxLogInfo(wxT("Found the Si570 USB device"));

	unsigned char version[2U];
	int n = ::libusb_control_transfer(m_device, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, REQUEST_GET_VERSION, 0xE00U, 0U, version, 2U, 500);
	if (n < 0) {
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), n);
		::libusb_close(m_device);
		return false;
	} else if (n == 2) {
		wxLogMessage(wxT("SI570Controller version: %u.%u"), version[1U], version[0U]);
		return true;
	} else {
		wxLogMessage(wxT("SI570Controller version: unknown"));
		return true;
	}
}

bool CSI570Controller::setFrequency(const CFrequency& freq)
{
	double dFrequency = double(freq.get()) / 1000000.0;

	wxUint32 frequency = wxUint32(dFrequency * (1UL << 21)) * m_freqMult;

	int n = ::libusb_control_transfer(m_device, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_OUT, REQUEST_SET_FREQUENCY, 0U, 0U, (unsigned char*)&frequency, 4U, 500);
	if (n < 0) {
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), n);
		return false;
	}

	return true;
}

bool CSI570Controller::setTransmit(bool tx)
{
	uint16_t value = tx ? 0x01U : 0x00U;

	unsigned char key;
	int n = ::libusb_control_transfer(m_device, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE | LIBUSB_ENDPOINT_IN, REQUEST_SET_TRANSMIT, value, 0U, &key, 1U, 500);
	if (n < 0) {
		wxLogError(wxT("Error from libusb_control_transfer: err=%d"), n);
		return false;
	}

	return true;
}

void CSI570Controller::close()
{
	::libusb_close(m_device);

	m_device = NULL;
}

#endif

void CSI570Controller::setCallback(IControlInterface* callback)
{
	wxASSERT(callback != NULL);

	m_callback = callback;
}

void CSI570Controller::enableTX(bool on)
{
	m_txEnable = on;
}

void CSI570Controller::enableRX(bool WXUNUSED(on))
{
}

void CSI570Controller::setTXAndFreq(bool transmit, const CFrequency& freq)
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
