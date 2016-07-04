/*
Copyright (c) 2012, Jared Boone <jared@sharebrained.com>
Copyright (c) 2013, Benjamin Vernoux <titanmkd@gmail.com>
Copyright (c) 2013, Michael Ossmann <mike@ossmann.com>
Copyright (c) 2013, Jonathan Naylor, G4KLX

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
Neither the name of Great Scott Gadgets nor the names of its contributors may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "LibHackRF.h"

#if defined(_WIN32)
#include <winusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#ifdef HACKRF_BIG_ENDIAN
#define TO_LE(x) __builtin_bswap32(x)
#else
#define TO_LE(x) x
#endif

// TODO: Factor this into a shared #include so that firmware can use
// the same values.
enum hackrf_vendor_request {
	HACKRF_VENDOR_REQUEST_SET_TRANSCEIVER_MODE = 1,
	HACKRF_VENDOR_REQUEST_MAX2837_WRITE = 2,
	HACKRF_VENDOR_REQUEST_MAX2837_READ = 3,
	HACKRF_VENDOR_REQUEST_SI5351C_WRITE = 4,
	HACKRF_VENDOR_REQUEST_SI5351C_READ = 5,
	HACKRF_VENDOR_REQUEST_SAMPLE_RATE_SET = 6,
	HACKRF_VENDOR_REQUEST_BASEBAND_FILTER_BANDWIDTH_SET = 7,
	HACKRF_VENDOR_REQUEST_RFFC5071_WRITE = 8,
	HACKRF_VENDOR_REQUEST_RFFC5071_READ = 9,
	HACKRF_VENDOR_REQUEST_SPIFLASH_ERASE = 10,
	HACKRF_VENDOR_REQUEST_SPIFLASH_WRITE = 11,
	HACKRF_VENDOR_REQUEST_SPIFLASH_READ = 12,
	HACKRF_VENDOR_REQUEST_CPLD_WRITE = 13,
	HACKRF_VENDOR_REQUEST_BOARD_ID_READ = 14,
	HACKRF_VENDOR_REQUEST_VERSION_STRING_READ = 15,
	HACKRF_VENDOR_REQUEST_SET_FREQ = 16,
	HACKRF_VENDOR_REQUEST_AMP_ENABLE = 17,
	HACKRF_VENDOR_REQUEST_BOARD_PARTID_SERIALNO_READ = 18,
	HACKRF_VENDOR_REQUEST_SET_LNA_GAIN = 19,
	HACKRF_VENDOR_REQUEST_SET_VGA_GAIN = 20,
	HACKRF_VENDOR_REQUEST_SET_TXVGA_GAIN = 21,
	HACKRF_VENDOR_REQUEST_SET_IF_FREQ = 22,
};

enum hackrf_transceiver_mode {
	HACKRF_TRANSCEIVER_MODE_OFF      = 0,
	HACKRF_TRANSCEIVER_MODE_RECEIVE  = 1,
	HACKRF_TRANSCEIVER_MODE_TRANSMIT = 2,
};

struct hackrf_device {
#if defined _WIN32
	WINUSB_INTERFACE_HANDLE handle;
	HANDLE file;
	HANDLE thread;
	OVERLAPPED* transfers;
#else
	libusb_device_handle*    usb_device;
	wxUint32                 transfer_count;
	struct libusb_transfer** transfers;
	pthread_t                transfer_thread;
#endif
	hackrf_sample_block_cb_fn callback;
	volatile bool transfer_thread_started; /* volatile shared between threads (read only) */
	wxUint32      buffer_size;
	volatile bool streaming; /* volatile shared between threads (read only) */
	void*         rx_ctx;
	void*         tx_ctx;
};

struct max2837_ft_t {
	wxUint32 bandwidth_hz;
};

const max2837_ft_t max2837_ft[] = {
	{ 1750000 },
	{ 2500000 },
	{ 3500000 },
	{ 5000000 },
	{ 5500000 },
	{ 6000000 },
	{ 7000000 },
	{ 8000000 },
	{ 9000000 },
	{ 10000000 },
	{ 12000000 },
	{ 14000000 },
	{ 15000000 },
	{ 20000000 },
	{ 24000000 },
	{ 28000000 },
	{ 0 }
};

volatile bool do_exit = false;

const wxUint16 hackrf_usb_vid = 0x1d50;
const wxUint16 hackrf_usb_pid = 0x604b;

#if !defined _WIN32
static libusb_context* g_libusb_context = NULL;
#endif

static void request_exit()
{
	do_exit = true;
}

static int cancel_transfers(hackrf_device* device)
{
	if (device->transfers == NULL)
		return HACKRF_ERROR_OTHER;

#if defined(_WIN32)
	::WinUsb_AbortPipe(device->handle, 0U);
#else
	for (wxUint32 transfer_index = 0U; transfer_index < device->transfer_count; transfer_index++) {
		if (device->transfers[transfer_index] != NULL)
			::libusb_cancel_transfer(device->transfers[transfer_index]);
	}
#endif

	return HACKRF_SUCCESS;
}

static int free_transfers(hackrf_device* device)
{
	if (device->transfers != NULL) {
#if !defined(_WIN32)
		// libusb_close() should free all transfers referenced from this array.
		for (wxUint32 transfer_index = 0U; transfer_index < device->transfer_count; transfer_index++) {
			if (device->transfers[transfer_index] != NULL) {
				::libusb_free_transfer(device->transfers[transfer_index]);
				device->transfers[transfer_index] = NULL;
			}
		}
#endif

		::free(device->transfers);
		device->transfers = NULL;
	}

	return HACKRF_SUCCESS;
}

static int allocate_transfers(hackrf_device* device)
{
	if (device->transfers != NULL)
		return HACKRF_ERROR_BUSY;

#if defined(_WIN32)
	device->transfers = (OVERLAPPED*)::calloc(1U, sizeof(OVERLAPPED));
	if (device->transfers == NULL)
		return HACKRF_ERROR_NO_MEM;

	device->transfers->hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
#else
	device->transfers = (libusb_transfer**)::calloc(device->transfer_count, sizeof(libusb_transfer));
	if (device->transfers == NULL)
		return HACKRF_ERROR_NO_MEM;

	for (wxUint32 transfer_index = 0U; transfer_index < device->transfer_count; transfer_index++) {
		device->transfers[transfer_index] = ::libusb_alloc_transfer(0);
		if (device->transfers[transfer_index] == NULL)
			return HACKRF_ERROR_LIBUSB;
	
		::libusb_fill_bulk_transfer(device->transfers[transfer_index], device->usb_device, 0, (unsigned char*)::malloc(device->buffer_size), device->buffer_size, NULL, device, 0);
	
		if (device->transfers[transfer_index]->buffer == NULL)
			return HACKRF_ERROR_NO_MEM;
	}
#endif

	return HACKRF_SUCCESS;
}

static int prepare_transfers(hackrf_device* device, wxUint8 endpoint_address, libusb_transfer_cb_fn callback)
{
	if (device->transfers == NULL)
		// This shouldn't happen.
		return HACKRF_ERROR_OTHER;

#if defined(_WIN32)
	::WinUsb_ReadPipe(device->handle, endpoint_address, (UCHAR*)::malloc(device->buffer_size), device->buffer_size, NULL, device->transfers);
#else
	for (wxUint32 transfer_index = 0U; transfer_index < device->transfer_count; transfer_index++) {
		device->transfers[transfer_index]->endpoint = endpoint_address;
		device->transfers[transfer_index]->callback = callback;

		int error = ::libusb_submit_transfer(device->transfers[transfer_index]);
		if (error != 0)
			return HACKRF_ERROR_LIBUSB;
	}
#endif

	return HACKRF_SUCCESS;
}

int hackrf_init()
{
#if defined(_WIN32)
	return HACKRF_SUCCESS;
#else
	int libusb_error = ::libusb_init(&g_libusb_context);

	if (libusb_error != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_exit()
{
#if !defined(_WIN32)
	if (g_libusb_context != NULL) {
		::libusb_exit(g_libusb_context);
		g_libusb_context = NULL;
	}
#endif

	return HACKRF_SUCCESS;
}

int hackrf_open(hackrf_device** device)
{
	if (device == NULL)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
#else
	// TODO: Do proper scanning of available devices, searching for
	// unit serial number (if specified?).
	libusb_device_handle* usb_device = ::libusb_open_device_with_vid_pid(g_libusb_context, hackrf_usb_vid, hackrf_usb_pid);
	if (usb_device == NULL)
		return HACKRF_ERROR_NOT_FOUND;

	//int speed = libusb_get_device_speed(usb_device);
	// TODO: Error or warning if not high speed USB?

	int result = ::libusb_set_configuration(usb_device, 1);
	if (result != 0) {
		::libusb_close(usb_device);
		return HACKRF_ERROR_LIBUSB;
	}

	result = ::libusb_claim_interface(usb_device, 0);
	if (result != 0) {
		::libusb_close(usb_device);
		return HACKRF_ERROR_LIBUSB;
	}

	hackrf_device* lib_device = (hackrf_device*)::malloc(sizeof(*lib_device));
	if (lib_device == NULL) {
		::libusb_release_interface(usb_device, 0);
		::libusb_close(usb_device);
		return HACKRF_ERROR_NO_MEM;
	}

	lib_device->usb_device = usb_device;
	lib_device->transfers = NULL;
	lib_device->callback = NULL;
	lib_device->transfer_thread_started = false;
	lib_device->transfer_count = 4;
	lib_device->buffer_size = 262144; /* 1048576; */
	lib_device->streaming = false;
	do_exit = false;

	result = allocate_transfers(lib_device);
	if (result != 0) {
		::free(lib_device);
		::libusb_release_interface(usb_device, 0);
		::libusb_close(usb_device);
		return HACKRF_ERROR_NO_MEM;
	}

	*device = lib_device;

	return HACKRF_SUCCESS;
#endif
}

int hackrf_set_transceiver_mode(hackrf_device* device, hackrf_transceiver_mode value)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SET_TRANSCEIVER_MODE;
	packet.Value = value;
	packet.Index = 0U;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SET_TRANSCEIVER_MODE, value, 0, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_max2837_read(hackrf_device* device, wxUint8 register_number, wxUint16* value)
{
	if (register_number >= 32U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_MAX2837_READ;
	packet.Value = 0U;
	packet.Index = register_number;
	packet.Length = 2U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR *)value, 2UL, &sent, NULL);

	if (!result || sent < 2UL)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_MAX2837_READ, 0, register_number, (unsigned char*)value, 2, 0);

	if (result < 2)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_max2837_write(hackrf_device* device, wxUint8 register_number, wxUint16 value)
{
	if (register_number >= 32U)
		return HACKRF_ERROR_INVALID_PARAM;

	if (value >= 0x400U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_MAX2837_WRITE;
	packet.Value = value;
	packet.Index = register_number;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_MAX2837_WRITE, value, register_number, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_si5351c_read(hackrf_device* device, wxUint16 register_number, wxUint16* value)
{
	if (register_number >= 256U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SI5351C_READ;
	packet.Value = 0U;
	packet.Index = register_number;
	packet.Length = 1U;

	wxUint8 temp_value = 0U;
	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, &temp_value, 1UL, &sent, NULL);

	if (!result || sent < 1U) {
		return HACKRF_ERROR_LIBUSB;
	} else {
		*value = temp_value;
		return HACKRF_SUCCESS;
	}
#else
	wxUint8 temp_value = 0U;
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SI5351C_READ, 0, register_number, (unsigned char*)&temp_value, 1, 0);

	if (result < 1) {
		return HACKRF_ERROR_LIBUSB;
	} else {
		*value = temp_value;
		return HACKRF_SUCCESS;
	}
#endif
}

int hackrf_si5351c_write(hackrf_device* device, wxUint16 register_number, wxUint16 value)
{
	if (register_number >= 256U)
		return HACKRF_ERROR_INVALID_PARAM;

	if (value >= 256U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SI5351C_WRITE;
	packet.Value = value;
	packet.Index = register_number;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SI5351C_WRITE, value, register_number, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_set_baseband_filter_bandwidth(hackrf_device* device, wxUint32 bandwidth_hz)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_BASEBAND_FILTER_BANDWIDTH_SET;
	packet.Value = bandwidth_hz & 0xffff;
	packet.Index = bandwidth_hz >> 16;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_BASEBAND_FILTER_BANDWIDTH_SET, bandwidth_hz & 0xffff, bandwidth_hz >> 16, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}


int hackrf_rffc5071_read(hackrf_device* device, wxUint8 register_number, wxUint16* value)
{
	if (register_number >= 31U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_RFFC5071_READ;
	packet.Value = 0U;
	packet.Index = register_number;
	packet.Length = 2U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR *)value, 2UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_RFFC5071_READ, 0, register_number, (unsigned char*)value, 2, 0);

	if (result < 2)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_rffc5071_write(hackrf_device* device, wxUint8 register_number, wxUint16 value)
{
	if (register_number >= 31U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_RFFC5071_WRITE;
	packet.Value = value;
	packet.Index = register_number;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_RFFC5071_WRITE, value, register_number, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_spiflash_erase(hackrf_device* device)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SPIFLASH_ERASE;
	packet.Value = 0U;
	packet.Index = 0U;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SPIFLASH_ERASE, 0, 0, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_spiflash_write(hackrf_device* device, wxUint32 address, wxUint16 length, const unsigned char* data)
{
	if (address > 0x0FFFFFU)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SPIFLASH_WRITE;
	packet.Value = address >> 16;
	packet.Index = address & 0xFFFF;
	packet.Length = length;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR*)data, length, &sent, NULL);

	if (!result || sent < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SPIFLASH_WRITE, address >> 16, address & 0xFFFF, data, length, 0);

	if (result < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_spiflash_read(hackrf_device* device, wxUint32 address, wxUint16 length, unsigned char* data)
{
	if (address > 0x0FFFFFU)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SPIFLASH_READ;
	packet.Value = address >> 16;
	packet.Index = address & 0xFFFF;
	packet.Length = length;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, data, length, &sent, NULL);

	if (!result || sent < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SPIFLASH_READ, address >> 16, address & 0xFFFF, data, length, 0);

	if (result < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_cpld_write(hackrf_device* device, wxUint16 length, const unsigned char* data, wxUint16 total_length)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_CPLD_WRITE;
	packet.Value = total_length;
	packet.Index = 0U;
	packet.Length = length;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR*)data, length, &sent, NULL);

	if (!result || sent < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_CPLD_WRITE, total_length, 0, data, length, 0);

	if (result < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_board_id_read(hackrf_device* device, wxUint8* value)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_BOARD_ID_READ;
	packet.Value = 0U;
	packet.Index = 0U;
	packet.Length = 1U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, value, 1UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_BOARD_ID_READ, 0, 0, value, 1, 0);

	if (result < 1)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_version_string_read(hackrf_device* device, char* version, wxUint8 length)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_VERSION_STRING_READ;
	packet.Value = 0U;
	packet.Index = 0U;
	packet.Length = length;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR *)version, length, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_VERSION_STRING_READ, 0, 0, (unsigned char*)version, length, 0);

	if (result < 0) {
		return HACKRF_ERROR_LIBUSB;
	} else {
		version[result] = '\0';
		return HACKRF_SUCCESS;
	}
#endif
}

typedef struct {
	wxUint32 freq_mhz; /* From 30 to 6000MHz */
	wxUint32 freq_hz; /* From 0 to 999999Hz */
	/* Final Freq = freq_mhz+freq_hz */
} set_freq_params_t;
#define FREQ_ONE_MHZ (1000*1000ull)

int hackrf_set_freq(hackrf_device* device, wxUint64 freq_hz)
{
	/* Convert Freq Hz 64bits to Freq MHz (32bits) & Freq Hz (32bits) */
	wxUint32 l_freq_mhz = (wxUint32)(freq_hz / FREQ_ONE_MHZ);
	wxUint32 l_freq_hz = (wxUint32)(freq_hz - (((wxUint64)l_freq_mhz) * FREQ_ONE_MHZ));

	set_freq_params_t set_freq_params;
	set_freq_params.freq_mhz = TO_LE(l_freq_mhz);
	set_freq_params.freq_hz = TO_LE(l_freq_hz);

#if defined(_WIN32)
	ULONG length = sizeof(set_freq_params_t);

	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SET_FREQ;
	packet.Value = 0U;
	packet.Index = 0U;
	packet.Length = USHORT(length);

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR *)&set_freq_params, length, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	wxUint8 length = sizeof(set_freq_params_t);

	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SET_FREQ, 0, 0, (unsigned char*)&set_freq_params, length, 0);

	if (result < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

typedef struct {
	wxUint32 freq_hz;
	wxUint32 divider;
} set_fracrate_params_t;

int hackrf_set_sample_rate_manual(hackrf_device* device, wxUint32 freq_hz, wxUint32 divider)
{
	set_fracrate_params_t set_fracrate_params;
	set_fracrate_params.freq_hz = TO_LE(freq_hz);
	set_fracrate_params.divider = TO_LE(divider);

#if defined(_WIN32)
	ULONG length = sizeof(set_fracrate_params_t);

	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SAMPLE_RATE_SET;
	packet.Value = 0U;
	packet.Index = 0U;
	packet.Length = length;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR *)&set_fracrate_params, length, &sent, NULL);

	if (!result || sent < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	wxUint8 length = sizeof(set_fracrate_params_t);

	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SAMPLE_RATE_SET, 0, 0, (unsigned char*)&set_fracrate_params, length, 0);

	if (result < length)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_set_sample_rate(hackrf_device* device, double freq)
{
	const unsigned int MAX_N = 32;
	double freq_frac = 1.0 + freq - int(freq);

	union {
		wxUint64 u64;
		double d;
	} v;
	v.d = freq;

	unsigned int e = (v.u64 >> 52) - 1023U;

	wxUint64 m = (1ULL << 52) - 1ULL;

	v.d = freq_frac;
	v.u64 &= m;

	m &= ~((1 << (e + 4)) - 1);

	wxUint64 a = 0ULL;

	unsigned int i = 1U;
	while (i < MAX_N) {
		a += v.u64;

		if (!(a & m) || !(~a & m))
			break;

		i++;
	}

	if (i == MAX_N)
		i = 1U;

	wxUint32 freq_hz = wxUint32(freq * i + 0.5);
	wxUint32 divider = i;

	return hackrf_set_sample_rate_manual(device, freq_hz, divider);
}

int hackrf_set_amp_enable(hackrf_device* device, wxUint8 value)
{
#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_AMP_ENABLE;
	packet.Value = value;
	packet.Index = 0U;
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_AMP_ENABLE, value, 0, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_board_partid_serialno_read(hackrf_device* device, read_partid_serialno_t* read_partid_serialno)
{
#if defined(_WIN32)
	ULONG length = sizeof(read_partid_serialno_t);

	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_BOARD_PARTID_SERIALNO_READ;
	packet.Value = 0U;
	packet.Index = 0U;
	packet.Length = USHORT(length);

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, (UCHAR *)read_partid_serialno, length, &sent, NULL);

	if (!result || sent < length) {
		return HACKRF_ERROR_LIBUSB;
	} else {
		read_partid_serialno->part_id[0] = TO_LE(read_partid_serialno->part_id[0]);
		read_partid_serialno->part_id[0] = TO_LE(read_partid_serialno->part_id[1]);
		read_partid_serialno->serial_no[0] = TO_LE(read_partid_serialno->serial_no[0]);
		read_partid_serialno->serial_no[1] = TO_LE(read_partid_serialno->serial_no[1]);
		read_partid_serialno->serial_no[2] = TO_LE(read_partid_serialno->serial_no[2]);
		read_partid_serialno->serial_no[3] = TO_LE(read_partid_serialno->serial_no[3]);

		return HACKRF_SUCCESS;
	}
#else
	wxUint8 length = sizeof(read_partid_serialno_t);

	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_BOARD_PARTID_SERIALNO_READ, 0, 0, (unsigned char*)read_partid_serialno, length, 0);

	if (result < length) {
		return HACKRF_ERROR_LIBUSB;
	} else {
		read_partid_serialno->part_id[0] = TO_LE(read_partid_serialno->part_id[0]);
		read_partid_serialno->part_id[0] = TO_LE(read_partid_serialno->part_id[1]);
		read_partid_serialno->serial_no[0] = TO_LE(read_partid_serialno->serial_no[0]);
		read_partid_serialno->serial_no[1] = TO_LE(read_partid_serialno->serial_no[1]);
		read_partid_serialno->serial_no[2] = TO_LE(read_partid_serialno->serial_no[2]);
		read_partid_serialno->serial_no[3] = TO_LE(read_partid_serialno->serial_no[3]);

		return HACKRF_SUCCESS;
	}
#endif
}

int hackrf_set_lna_gain(hackrf_device* device, wxUint32 value)
{
	if (value > 40U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SET_LNA_GAIN;
	packet.Value = 0U;
	packet.Index = USHORT(value);
	packet.Length = 1U;

	wxUint8 retval;
	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, &retval, 1UL, &sent, NULL);

	if (!result || sent != 1U)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	wxUint8 retval;
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SET_LNA_GAIN, 0, value, &retval, 1, 0);

	if (result != 1 || !retval)
		return HACKRF_ERROR_INVALID_PARAM;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_set_vga_gain(hackrf_device* device, wxUint32 value)
{
	if (value > 62U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SET_VGA_GAIN;
	packet.Value = 0U;
	packet.Index = USHORT(value);
	packet.Length = 1U;

	wxUint8 retval;
	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, &retval, 1UL, &sent, NULL);

	if (!result || sent != 1U)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	wxUint8 retval;
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SET_VGA_GAIN, 0, value, &retval, 1, 0);

	if (result != 1 || !retval)
		return HACKRF_ERROR_INVALID_PARAM;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_set_txvga_gain(hackrf_device* device, wxUint32 value)
{
	if (value > 47U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_DEVICE_TO_HOST << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SET_TXVGA_GAIN;
	packet.Value = 0U;
	packet.Index = USHORT(value);
	packet.Length = 1U;

	wxUint8 retval;
	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, &retval, 1UL, &sent, NULL);

	if (!result || sent != 1U)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	wxUint8 retval;
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SET_TXVGA_GAIN, 0, value, &retval, 1, 0);

	if (result != 1 || !retval)
		return HACKRF_ERROR_INVALID_PARAM;
	else
		return HACKRF_SUCCESS;
#endif
}

int hackrf_set_if_freq(hackrf_device* device, wxUint32 freq_mhz)
{
	if (freq_mhz < 2300U || freq_mhz > 2700U)
		return HACKRF_ERROR_INVALID_PARAM;

#if defined(_WIN32)
	WINUSB_SETUP_PACKET packet;
	packet.RequestType = (BMREQUEST_HOST_TO_DEVICE << 7) | (BMREQUEST_VENDOR << 5) | BMREQUEST_TO_DEVICE;
	packet.Request = HACKRF_VENDOR_REQUEST_SET_IF_FREQ;
	packet.Value = 0U;
	packet.Index = USHORT(freq_mhz);
	packet.Length = 0U;

	ULONG sent = 0UL;
	BOOL result = ::WinUsb_ControlTransfer(device->handle, packet, NULL, 0UL, &sent, NULL);

	if (!result)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#else
	int result = ::libusb_control_transfer(device->usb_device, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, HACKRF_VENDOR_REQUEST_SET_IF_FREQ, 0, freq_mhz, NULL, 0, 0);

	if (result != 0)
		return HACKRF_ERROR_LIBUSB;
	else
		return HACKRF_SUCCESS;
#endif
}

static void* transfer_threadproc(void* arg)
{
	hackrf_device* device = (hackrf_device*)arg;
	while (device->streaming && !do_exit) {
		struct timeval timeout = {0, 500000};
		int error = ::libusb_handle_events_timeout(g_libusb_context, &timeout);
		if (error != 0 && error != LIBUSB_ERROR_INTERRUPTED)
			device->streaming = false;
	}

	return NULL;
}

#if !defined(_WIN32)
static void hackrf_libusb_transfer_callback(libusb_transfer* usb_transfer)
{
	hackrf_device* device = (hackrf_device*)usb_transfer->user_data;

	if (usb_transfer->status == LIBUSB_TRANSFER_COMPLETED) {
		hackrf_transfer transfer = {
			transfer.device = device,
			transfer.buffer = usb_transfer->buffer,
			transfer.buffer_length = usb_transfer->length,
			transfer.valid_length = usb_transfer->actual_length,
			transfer.rx_ctx = device->rx_ctx,
			transfer.tx_ctx = device->tx_ctx
		};

		if (device->callback(&transfer) == 0) {
			if (::libusb_submit_transfer(usb_transfer) < 0)
				request_exit();
			else
				return;
		} else {
			request_exit();
		}
	} else {
		/* Other cases LIBUSB_TRANSFER_NO_DEVICE
		LIBUSB_TRANSFER_ERROR, LIBUSB_TRANSFER_TIMED_OUT
		LIBUSB_TRANSFER_STALL, LIBUSB_TRANSFER_OVERFLOW
		LIBUSB_TRANSFER_CANCELLED ...
		*/

		request_exit(); /* Fatal error stop transfer */
	}
}
#endif

static int kill_transfer_thread(hackrf_device* device)
{
	request_exit();

	if (device->transfer_thread_started) {
		void* value = NULL;
		int result = ::pthread_join(device->transfer_thread, &value);
		if (result != 0)
			return HACKRF_ERROR_THREAD;

		device->transfer_thread_started = false;

		/* Cancel all transfers */
		cancel_transfers(device);
	}

	return HACKRF_SUCCESS;
}

static int create_transfer_thread(hackrf_device* device, wxUint8 endpoint_address, hackrf_sample_block_cb_fn callback)
{
	if (device->transfer_thread_started)
		return HACKRF_ERROR_BUSY;

	device->streaming = false;

	int result = prepare_transfers(device, endpoint_address, (libusb_transfer_cb_fn)hackrf_libusb_transfer_callback);
	if (result != HACKRF_SUCCESS)
		return result;

	device->streaming = true;
	device->callback = callback;

	result = ::pthread_create(&device->transfer_thread, 0, transfer_threadproc, device);
	if (result == 0)
		device->transfer_thread_started = true;
	else
		return HACKRF_ERROR_THREAD;

	return HACKRF_SUCCESS;
}

int hackrf_is_streaming(hackrf_device* device)
{
	/* return hackrf is streaming only when streaming, transfer_thread_started are true and do_exit equal false */
	if (device->transfer_thread_started && device->streaming && !do_exit) {
		return HACKRF_TRUE;
	} else {
		if (!device->transfer_thread_started)
			return HACKRF_ERROR_STREAMING_THREAD_ERR;

		if (!device->streaming)
			return HACKRF_ERROR_STREAMING_STOPPED;

		return HACKRF_ERROR_STREAMING_EXIT_CALLED;
	}
}

int hackrf_start_rx(hackrf_device* device, hackrf_sample_block_cb_fn callback, void* rx_ctx)
{
	const wxUint8 endpoint_address = LIBUSB_ENDPOINT_IN | 1;

	int result = hackrf_set_transceiver_mode(device, HACKRF_TRANSCEIVER_MODE_RECEIVE);

	if (result == HACKRF_SUCCESS) {
		device->rx_ctx = rx_ctx;
		result = create_transfer_thread(device, endpoint_address, callback);
	}

	return result;
}

int hackrf_stop_rx(hackrf_device* device)
{
	int result1 = kill_transfer_thread(device);
	int result2 = hackrf_set_transceiver_mode(device, HACKRF_TRANSCEIVER_MODE_OFF);

	if (result2 != HACKRF_SUCCESS)
		return result2;

	return result1;
}

int hackrf_start_tx(hackrf_device* device, hackrf_sample_block_cb_fn callback, void* tx_ctx)
{
	const wxUint8 endpoint_address = LIBUSB_ENDPOINT_OUT | 2;

	int result = hackrf_set_transceiver_mode(device, HACKRF_TRANSCEIVER_MODE_TRANSMIT);

	if (result == HACKRF_SUCCESS) {
		device->tx_ctx = tx_ctx;
		result = create_transfer_thread(device, endpoint_address, callback);
	}

	return result;
}

int hackrf_stop_tx(hackrf_device* device)
{
	int result1 = kill_transfer_thread(device);
	int result2 = hackrf_set_transceiver_mode(device, HACKRF_TRANSCEIVER_MODE_OFF);

	if (result2 != HACKRF_SUCCESS)
		return result2;

	return result1;
}

int hackrf_close(hackrf_device* device)
{
	int result1 = HACKRF_SUCCESS;
	int result2 = HACKRF_SUCCESS;

	if (device != NULL) {
		result1 = hackrf_stop_rx(device);
		result2 = hackrf_stop_tx(device);

#if defined(_WIN32)
		::WinUsb_Free(device->handle);
		::CloseHandle(device->file);
		device->handle = INVALID_HANDLE_VALUE;
		device->file = INVALID_HANDLE_VALUE;
#else
		if (device->usb_device != NULL) {
			::libusb_release_interface(device->usb_device, 0);
			::libusb_close(device->usb_device);
			device->usb_device = NULL;
		}
#endif
		free_transfers(device);
		::free(device);
	}

	if (result2 != HACKRF_SUCCESS)
		return result2;

	return result1;
}

const char* hackrf_error_name(hackrf_error errcode)
{
	switch (errcode) {
		case HACKRF_SUCCESS:
			return "HACKRF_SUCCESS";

		case HACKRF_TRUE:
			return "HACKRF_TRUE";

		case HACKRF_ERROR_INVALID_PARAM:
			return "HACKRF_ERROR_INVALID_PARAM";

		case HACKRF_ERROR_NOT_FOUND:
			return "HACKRF_ERROR_NOT_FOUND";

		case HACKRF_ERROR_BUSY:
			return "HACKRF_ERROR_BUSY";

		case HACKRF_ERROR_NO_MEM:
			return "HACKRF_ERROR_NO_MEM";

		case HACKRF_ERROR_LIBUSB:
			return "HACKRF_ERROR_LIBUSB";

		case HACKRF_ERROR_THREAD:
			return "HACKRF_ERROR_THREAD";

		case HACKRF_ERROR_STREAMING_THREAD_ERR:
			return "HACKRF_ERROR_STREAMING_THREAD_ERR";

		case HACKRF_ERROR_STREAMING_STOPPED:
			return "HACKRF_ERROR_STREAMING_STOPPED";

		case HACKRF_ERROR_STREAMING_EXIT_CALLED:
			return "HACKRF_ERROR_STREAMING_EXIT_CALLED";

		case HACKRF_ERROR_OTHER:
			return "HACKRF_ERROR_OTHER";

		default:
			return "HACKRF unknown error";
	}
}

const char* hackrf_board_id_name(hackrf_board_id board_id)
{
	switch (board_id) {
		case BOARD_ID_JELLYBEAN:
			return "Jellybean";
	
		case BOARD_ID_JAWBREAKER:
			return "Jawbreaker";
	
		case BOARD_ID_INVALID:
			return "Invalid Board ID";
	
		default:
			return "Unknown Board ID";
	}
}

/* Return final bw round down and less than expected bw. */
wxUint32 hackrf_compute_baseband_filter_bw_round_down_lt(wxUint32 bandwidth_hz)
{
	const max2837_ft_t* p = max2837_ft;

	while (p->bandwidth_hz != 0U) {
		if (p->bandwidth_hz >= bandwidth_hz )
			break;

		p++;
	}

	/* Round down (if no equal to first entry) */
	if (p != max2837_ft)
		p--;

	return p->bandwidth_hz;
}

/* Return final bw. */
wxUint32 hackrf_compute_baseband_filter_bw(wxUint32 bandwidth_hz)
{
	const max2837_ft_t* p = max2837_ft;

	while (p->bandwidth_hz != 0U) {
		if (p->bandwidth_hz >= bandwidth_hz)
			break;

		p++;
	}
	
	/* Round down (if no equal to first entry) and if > bandwidth_hz */
	if (p != max2837_ft) {
		if (p->bandwidth_hz > bandwidth_hz)
			p--;
	}
	
	return p->bandwidth_hz;
}
