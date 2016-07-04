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

#ifndef __LIBHACKRF_H__
#define __LIBHACKRF_H__

#include <wx/wx.h>

enum hackrf_error {
	HACKRF_SUCCESS = 0,
	HACKRF_TRUE = 1,
	HACKRF_ERROR_INVALID_PARAM = -2,
	HACKRF_ERROR_NOT_FOUND = -5,
	HACKRF_ERROR_BUSY = -6,
	HACKRF_ERROR_NO_MEM = -11,
	HACKRF_ERROR_LIBUSB = -1000,
	HACKRF_ERROR_THREAD = -1001,
	HACKRF_ERROR_STREAMING_THREAD_ERR = -1002,
	HACKRF_ERROR_STREAMING_STOPPED = -1003,
	HACKRF_ERROR_STREAMING_EXIT_CALLED = -1004,
	HACKRF_ERROR_OTHER = -9999,
};

enum hackrf_board_id {
	BOARD_ID_JELLYBEAN = 0,
	BOARD_ID_JAWBREAKER = 1,
	BOARD_ID_INVALID = 0xFF,
};

struct hackrf_device;

struct hackrf_transfer {
	hackrf_device* device;
	wxUint8* buffer;
	int buffer_length;
	int valid_length;
	void* rx_ctx;
	void* tx_ctx;
};

struct read_partid_serialno_t {
	wxUint32 part_id[2];
	wxUint32 serial_no[4];
};

typedef int (*hackrf_sample_block_cb_fn)(hackrf_transfer* transfer);

extern int hackrf_init();
extern int hackrf_exit();
 
extern int hackrf_open(hackrf_device** device);
extern int hackrf_close(hackrf_device* device);
 
extern int hackrf_start_rx(hackrf_device* device, hackrf_sample_block_cb_fn callback, void* rx_ctx);
extern int hackrf_stop_rx(hackrf_device* device);
 
extern int hackrf_start_tx(hackrf_device* device, hackrf_sample_block_cb_fn callback, void* tx_ctx);
extern int hackrf_stop_tx(hackrf_device* device);

/* return HACKRF_TRUE if success */
extern int hackrf_is_streaming(hackrf_device* device);
 
extern int hackrf_max2837_read(hackrf_device* device, wxUint8 register_number, wxUint16* value);
extern int hackrf_max2837_write(hackrf_device* device, wxUint8 register_number, wxUint16 value);
 
extern int hackrf_si5351c_read(hackrf_device* device, wxUint16 register_number, wxUint16* value);
extern int hackrf_si5351c_write(hackrf_device* device, wxUint16 register_number, wxUint16 value);
 
extern int hackrf_set_baseband_filter_bandwidth(hackrf_device* device, wxUint32 bandwidth_hz);
 
extern int hackrf_rffc5071_read(hackrf_device* device, wxUint8 register_number, wxUint16* value);
extern int hackrf_rffc5071_write(hackrf_device* device, wxUint8 register_number, wxUint16 value);
 
extern int hackrf_spiflash_erase(hackrf_device* device);
extern int hackrf_spiflash_write(hackrf_device* device, wxUint32 address, wxUint16 length, const unsigned char* data);
extern int hackrf_spiflash_read(hackrf_device* device, wxUint32 address, wxUint16 length, unsigned char* data);

extern int hackrf_cpld_write(hackrf_device* device, wxUint16 length,
unsigned char* const data, wxUint16 total_length);

extern int hackrf_board_id_read(hackrf_device* device, wxUint8* value);
extern int hackrf_version_string_read(hackrf_device* device, char* version, wxUint8 length);

extern int hackrf_set_freq(hackrf_device* device, wxUint64 freq_hz);

/* range 2300-2700 Mhz */
extern int hackrf_set_if_freq(hackrf_device* device, wxUint32 freq_mhz);

/* currently 8-20Mhz - either as a fraction, i.e. freq 20000000hz divider 2 -> 10Mhz or as plain old 10000000hz (double)
preferred rates are 8, 10, 12.5, 16, 20Mhz due to less jitter */
extern int hackrf_set_sample_rate_manual(hackrf_device* device, wxUint32 freq_hz, wxUint32 divider);
extern int hackrf_set_sample_rate(hackrf_device* device, double freq_hz);

/* external amp, bool on/off */
extern int hackrf_set_amp_enable(hackrf_device* device, wxUint8 value);

extern int hackrf_board_partid_serialno_read(hackrf_device* device, read_partid_serialno_t* read_partid_serialno);

/* range 0-40 step 8db */
extern int hackrf_set_lna_gain(hackrf_device* device, wxUint32 value);

/* range 0-62 step 2db */
extern int hackrf_set_vga_gain(hackrf_device* device, wxUint32 value);

/* range 0-47 step 1db */
extern int hackrf_set_txvga_gain(hackrf_device* device, wxUint32 value);

extern const char* hackrf_error_name(hackrf_error errcode);
extern const char* hackrf_board_id_name(hackrf_board_id board_id);

/* Compute nearest freq for bw filter (manual filter) */
extern wxUint32 hackrf_compute_baseband_filter_bw_round_down_lt(wxUint32 bandwidth_hz);
/* Compute best default value depending on sample rate (auto filter) */
extern wxUint32 hackrf_compute_baseband_filter_bw(wxUint32 bandwidth_hz);

#endif//__LIBHACKRF_H__
