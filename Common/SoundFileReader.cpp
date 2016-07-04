/*
 *   Copyright (C) 2002-2004,2006-2008,2013 by Jonathan Naylor G4KLX
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

#include "SoundFileReader.h"

#include <wx/debug.h>
#include <wx/log.h>
#include <wx/datetime.h>


void CSoundFileReader::setCallback(IDataCallback* callback, int id)
{
	m_callback = callback;
	m_id       = id;
}

#if defined(__WINDOWS__)

const int WAVE_FORMAT_IEEE_FLOAT = 3;

CSoundFileReader::CSoundFileReader(const wxString& fileName, IDataReader* reader) :
CThreadReader(reader),
m_fileName(fileName),
m_blockSize(0U),
m_callback(NULL),
m_id(0),
m_buffer(NULL),
m_format(99U),
m_channels(0U),
m_buffer8(NULL),
m_buffer16(NULL),
m_buffer32(NULL),
m_handle(NULL),
m_parent(),
m_child(),
m_offset(0L)
{
}

CSoundFileReader::~CSoundFileReader()
{
	if (m_handle != NULL)
		::mmioClose(m_handle, 0);

	delete[] m_buffer;
	delete[] m_buffer8;
	delete[] m_buffer16;
	delete[] m_buffer32;
}

bool CSoundFileReader::open(float sampleRate, unsigned int blockSize)
{
	m_blockSize = blockSize;

	m_handle = ::mmioOpen(LPWSTR(m_fileName.c_str()), 0, MMIO_READ | MMIO_ALLOCBUF);

	if (m_handle == NULL) {
		wxLogError(wxT("SoundFileReader: could not open the WAV file %s."), m_fileName.c_str());
		return false;
	}

	MMCKINFO parent;
	parent.fccType = mmioFOURCC('W', 'A', 'V', 'E');

	MMRESULT res = ::mmioDescend(m_handle, &parent, 0, MMIO_FINDRIFF);

	if (res != MMSYSERR_NOERROR) {
		wxLogError(wxT("SoundFileReader: %s has no \"WAVE\" header."), m_fileName.c_str());
		return false;
	}

	MMCKINFO child;
	child.ckid = mmioFOURCC('f', 'm', 't', ' ');

	res = ::mmioDescend(m_handle, &child, &parent, MMIO_FINDCHUNK);

	if (res != MMSYSERR_NOERROR) {
		wxLogError(wxT("SoundFileReader: %s has no \"fmt \" chunk."), m_fileName.c_str());
		return false;
	}

	WAVEFORMATEX format;

	LONG len = ::mmioRead(m_handle, (char *)&format, child.cksize);

	if (len != LONG(child.cksize)) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the WAVEFORMATEX structure."), m_fileName.c_str());
		return false;
	}
	
	if (format.wFormatTag != WAVE_FORMAT_PCM && format.wFormatTag != WAVE_FORMAT_IEEE_FLOAT) {
		wxLogError(wxT("SoundFileReader: %s is not PCM or IEEE Float format, is %u."), m_fileName.c_str(), format.wFormatTag);
		return false;
	}

	if (format.nSamplesPerSec != DWORD(sampleRate)) {
		wxLogError(wxT("SoundFileReader: %s has sample rate %lu, not %.0f"), m_fileName.c_str(), format.nSamplesPerSec, sampleRate);
		return false;
	}

	m_channels = format.nChannels;
	if (m_channels > 2U) {
		wxLogError(wxT("SoundFileReader: %s has %u channels, more than 2."), m_fileName.c_str(), m_channels);
		return false;
	}

	if (format.wBitsPerSample == 8U && format.wFormatTag == WAVE_FORMAT_PCM) {
		m_format = FORMAT_8BIT;
	} else if (format.wBitsPerSample == 16U && format.wFormatTag == WAVE_FORMAT_PCM) {
		m_format = FORMAT_16BIT;
	} else if (format.wBitsPerSample == 32U && format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
		m_format = FORMAT_32BIT;
	} else {
		wxLogError(wxT("SoundFileReader: %s has sample width %u and format %u."), m_fileName.c_str(), format.wBitsPerSample, format.wFormatTag);
		return false;
	}

	res = ::mmioAscend(m_handle, &child, 0);

	if (res != MMSYSERR_NOERROR) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot ascend."), m_fileName.c_str());
		return false;
	}

	child.ckid = mmioFOURCC('d', 'a', 't', 'a');

	res = ::mmioDescend(m_handle, &child, &parent, MMIO_FINDCHUNK);

	if (res != MMSYSERR_NOERROR) {
		wxLogError(wxT("SoundFileReader: %s has no \"data\" chunk."), m_fileName.c_str());
		return false;
	}

	// Get the current location so we can rewind if needed
	m_offset = ::mmioSeek(m_handle, 0L, SEEK_CUR);

	switch (m_format) {
		case FORMAT_8BIT:
			m_buffer8 =  new wxUint8[m_blockSize * m_channels];
			break;
		case FORMAT_16BIT:
			m_buffer16 = new wxInt16[m_blockSize * m_channels];
			break;
		case FORMAT_32BIT:
			m_buffer32 = new wxFloat32[m_blockSize * m_channels];
			break;
	}

	m_buffer = new float[m_blockSize * 2U];

	return CThreadReader::open(sampleRate, blockSize);
}

bool CSoundFileReader::create()
{
	wxASSERT(m_callback != NULL);
	wxASSERT(m_handle != NULL);

	LONG n = 0L;
	LONG i;

	switch (m_format) {
		case FORMAT_8BIT:
			n = ::mmioRead(m_handle, (char *)m_buffer8, m_blockSize * m_channels * sizeof(wxUint8));

			if (n <= 0L) {
				m_callback->callback(m_buffer, 0U, m_id);
				return false;
			}

			n /= (sizeof(wxUint8) * m_channels);

			switch (m_channels) {
				case 1U:
					for (i = 0L; i < n; i++)
						m_buffer[i * 2U + 0U] = m_buffer[i * 2U + 1U] = (float(m_buffer8[i]) - 127.0F) / 128.0F;
					break;
				case 2U:
					for (i = 0L; i < n; i++) {
						m_buffer[i * 2U + 0U] = (float(m_buffer8[i * 2U + 0U]) - 127.0F) / 128.0F;
						m_buffer[i * 2U + 1U] = (float(m_buffer8[i * 2U + 1U]) - 127.0F) / 128.0F;
					}
					break;
			}
			break;

		case FORMAT_16BIT:
			n = ::mmioRead(m_handle, (char *)m_buffer16, m_blockSize * m_channels * sizeof(wxInt16));

			if (n <= 0L) {
				m_callback->callback(m_buffer, 0U, m_id);
				return false;
			}

			n /= (sizeof(wxInt16) * m_channels);

			switch (m_channels) {
				case 1U:
					for (i = 0L; i < n; i++)
						m_buffer[i * 2U + 0U] = m_buffer[i * 2U + 1U] = float(m_buffer16[i]) / 32768.0F;
					break;
				case 2U:
					for (i = 0L; i < n; i++) {
						m_buffer[i * 2U + 0U] = float(m_buffer16[i * 2U + 0U]) / 32768.0F;
						m_buffer[i * 2U + 1U] = float(m_buffer16[i * 2U + 1U]) / 32768.0F;
					}
					break;
			}
			break;

		case FORMAT_32BIT:
			n = ::mmioRead(m_handle, (char *)m_buffer32, m_blockSize * m_channels * sizeof(wxFloat32));

			if (n <= 0L) {
				m_callback->callback(m_buffer, 0U, m_id);
				return false;
			}

			n /= (sizeof(wxFloat32) * m_channels);

			switch (m_channels) {
				case 1U:
					for (i = 0L; i < n; i++)
						m_buffer[i * 2U + 0U] = m_buffer32[i * 2U + 1U];
					break;
				case 2U:
					// Swap I and Q for SDR-1000 data
					for (i = 0L; i < n; i++) {
						m_buffer[i * 2U + 0U] = m_buffer32[i * 2U + 1U];
						m_buffer[i * 2U + 1U] = m_buffer32[i * 2U + 0U];
					}
					break;
			}
	}

	m_callback->callback(m_buffer, n, m_id);

	return true;
}

void CSoundFileReader::rewind()
{
	wxASSERT(m_handle != NULL);

	::mmioSeek(m_handle, m_offset, SEEK_SET);
}

#else

const int FORMAT_PCM        = 1;
const int FORMAT_IEEE_FLOAT = 3;

CSoundFileReader::CSoundFileReader(const wxString& fileName, IDataReader* reader) :
CThreadReader(reader),
m_fileName(fileName),
m_blockSize(0U),
m_callback(NULL),
m_id(0),
m_buffer(NULL),
m_format(99U),
m_channels(0U),
m_buffer8(NULL),
m_buffer16(NULL),
m_buffer32(NULL),
m_file(NULL),
m_offset(0),
m_length(0U),
m_read(0U)
{
}

CSoundFileReader::~CSoundFileReader()
{
	if (m_file != NULL) {
		m_file->Close();
		delete m_file;
	}

	delete[] m_buffer;
	delete[] m_buffer8;
	delete[] m_buffer16;
	delete[] m_buffer32;
}

bool CSoundFileReader::open(float sampleRate, unsigned int blockSize)
{
	m_blockSize = blockSize;
	m_read      = 0U;

	m_file = new wxFFile(m_fileName.c_str(), wxT("rb"));

	bool ret = m_file->IsOpened();
	if (!ret) {
		wxLogError(wxT("SoundFileReader: could not open the WAV file %s."), m_fileName.c_str());

		delete m_file;
		m_file = NULL;

		return false;
	}

	unsigned char buffer[4];

	unsigned int n = m_file->Read(buffer, 4);
	if (n != 4U || ::memcmp(buffer, "RIFF", 4) != 0) {
		wxLogError(wxT("SoundFileReader: %s has no \"RIFF\" signature."), m_fileName.c_str());
		return false;
	}

	n = m_file->Read(buffer, 4);
	if (n != 4U) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the file length."), m_fileName.c_str());
		return false;
	}

	n = m_file->Read(buffer, 4);
	if (n != 4U || ::memcmp(buffer, "WAVE", 4) != 0) {
		wxLogError(wxT("SoundFileReader: %s has no \"WAVE\" header."), m_fileName.c_str());
		return false;
	}

	n = m_file->Read(buffer, 4);
	if (n != 4U || ::memcmp(buffer, "fmt ", 4) != 0) {
		wxLogError(wxT("SoundFileReader: %s has no \"fmt \" chunk."), m_fileName.c_str());
		return false;
	}

	wxUint32 uint32;
	n = m_file->Read(&uint32, sizeof(wxUint32));

	wxUint32 length = wxUINT32_SWAP_ON_BE(uint32);
	if (n != sizeof(wxUint32) || length < 16U) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the WAVEFORMATEX structure length."), m_fileName.c_str());
		return false;
	}

	wxUint16 uint16;
	n = m_file->Read(&uint16, sizeof(wxUint16));

	wxUint16 compCode = wxUINT16_SWAP_ON_BE(uint16);
	if (n != sizeof(wxUint16) || (compCode != FORMAT_PCM && compCode != FORMAT_IEEE_FLOAT)) {
		wxLogError(wxT("SoundFileReader: %s is not PCM or IEEE Float format, is %u."), m_fileName.c_str(), compCode);
		return false;
	}

	n = m_file->Read(&uint16, sizeof(wxUint16));

	m_channels = wxUINT16_SWAP_ON_BE(uint16);
	if (n != sizeof(wxUint16) || m_channels > 2U) {
		wxLogError(wxT("SoundFileReader: %s has %u channels, more than 2."), m_fileName.c_str(), m_channels);
		return false;
	}


	n = m_file->Read(&uint32, sizeof(wxUint32));

	wxUint32 samplesPerSec = wxUINT32_SWAP_ON_BE(uint32);
	if (n != sizeof(wxUint32) || samplesPerSec != sampleRate) {
		wxLogError(wxT("SoundFileReader: %s has sample rate %lu, not %.0f"), m_fileName.c_str(), (unsigned long)samplesPerSec, sampleRate);
		return false;
	}

	n = m_file->Read(&uint32, sizeof(wxUint32));

	if (n != sizeof(wxUint32)) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the average bytes per second"), m_fileName.c_str());
		return false;
	}

	n = m_file->Read(&uint16, sizeof(wxUint16));

	if (n != sizeof(wxUint16)) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the block align."), m_fileName.c_str());
		return false;
	}

	n = m_file->Read(&uint16, sizeof(wxUint16));

	if (n != sizeof(wxUint16)) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the bitsPerSample."), m_fileName.c_str());
		return false;
	}

	wxUint16 bitsPerSample = wxUINT16_SWAP_ON_BE(uint16);

	if (bitsPerSample == 8U && compCode == FORMAT_PCM) {
		m_format = FORMAT_8BIT;
	} else if (bitsPerSample == 16U && compCode == FORMAT_PCM) {
		m_format = FORMAT_16BIT;
	} else if (bitsPerSample == 32U && compCode == FORMAT_IEEE_FLOAT) {
		m_format = FORMAT_32BIT;
	} else {
		wxLogError(wxT("SoundFileReader: %s has sample width %u and format %u."), m_fileName.c_str(), bitsPerSample, compCode);
		return false;
	}

	// Now drain any extra bytes of data
	if (length > 16U)
		m_file->Seek(length - 16U, wxFromCurrent);

	n = m_file->Read(buffer, 4);

	if (n != 4U || ::memcmp(buffer, "data", 4) != 0) {
		wxLogError(wxT("SoundFileReader: %s has no \"data\" chunk."), m_fileName.c_str());
		return false;
	}

	n = m_file->Read(&uint32, sizeof(wxUint32));

	if (n != sizeof(wxUint32)) {
		wxLogError(wxT("SoundFileReader: %s is corrupt, cannot read the \"data\" chunk size"), m_fileName.c_str());
		return false;
	}

	m_length = wxUINT32_SWAP_ON_BE(uint32);

	// Get the current location so we can rewind if needed
	m_offset = m_file->Tell();

	switch (m_format) {
		case FORMAT_8BIT:
			m_buffer8 =  new wxUint8[m_blockSize * m_channels];
			break;
		case FORMAT_16BIT:
			m_buffer16 = new wxInt16[m_blockSize * m_channels];
			break;
		case FORMAT_32BIT:
			m_buffer32 = new wxFloat32[m_blockSize * m_channels];
			break;
	}

	m_buffer = new float[m_blockSize * 2U];

	return CThreadReader::open(sampleRate, blockSize);
}

bool CSoundFileReader::create()
{
	wxASSERT(m_callback != NULL);
	wxASSERT(m_file != NULL);

	unsigned int i;
	unsigned int readSize;
	size_t n = 0U;

	switch (m_format) {
		case FORMAT_8BIT:
			readSize = m_blockSize * m_channels * sizeof(wxUint8);

			if (readSize > (m_length - m_read))
				readSize = (m_length - m_read) / (m_channels * sizeof(wxUint8));

			n = m_file->Read(m_buffer8, readSize);

			if (n == 0U) {
				m_callback->callback(m_buffer, 0U, m_id);
				return false;
			}

			m_read += n;

			n /= (sizeof(wxUint8) * m_channels);

			switch (m_channels) {
				case 1U:
					for (i = 0U; i < n; i++)
						m_buffer[i * 2U + 0U] = m_buffer[i * 2U + 1U] = (float(m_buffer8[i]) - 127.0F) / 128.0F;
					break;
				case 2U:
					for (i = 0U; i < n; i++) {
						m_buffer[i * 2U + 0U] = (float(m_buffer8[i * 2U + 0U]) - 127.0F) / 128.0F;
						m_buffer[i * 2U + 1U] = (float(m_buffer8[i * 2U + 1U]) - 127.0F) / 128.0F;
					}
					break;
			}
			break;

		case FORMAT_16BIT:
			readSize = m_blockSize * m_channels * sizeof(wxInt16);

			if (readSize > (m_length - m_read))
				readSize = (m_length - m_read) / (m_channels * sizeof(wxInt16));

			n = m_file->Read(m_buffer16, readSize);

			if (n == 0U) {
				m_callback->callback(m_buffer, 0U, m_id);
				return false;
			}

			m_read += n;

			n /= (sizeof(wxInt16) * m_channels);

			switch (m_channels) {
				case 1U:
					for (i = 0U; i < n; i++)
						m_buffer[i * 2U + 0U] = m_buffer[i * 2U + 1U] = float(m_buffer16[i]) / 32768.0F;
					break;
				case 2U:
					for (i = 0U; i < n; i++) {
						m_buffer[i * 2U + 0U] = float(m_buffer16[i * 2U + 0U]) / 32768.0F;
						m_buffer[i * 2U + 1U] = float(m_buffer16[i * 2U + 1U]) / 32768.0F;
					}
					break;
			}
			break;

		case FORMAT_32BIT:
			readSize = m_blockSize * m_channels * sizeof(wxFloat32);

			if (readSize > (m_length - m_read))
				readSize = (m_length - m_read) / (m_channels * sizeof(wxFloat32));

			n = m_file->Read(m_buffer32, readSize);

			if (n == 0U) {
				m_callback->callback(m_buffer, 0U, m_id);
				return false;
			}

			m_read += n;

			n /= (sizeof(wxFloat32) * m_channels);

			switch (m_channels) {
				case 1U:
					for (i = 0U; i < n; i++)
						m_buffer[i * 2U + 0U] = m_buffer32[i * 2U + 1U];
					break;
				case 2U:
					// Swap I and Q for SDR-1000 data
					for (i = 0U; i < n; i++) {
						m_buffer[i * 2U + 0U] = m_buffer32[i * 2U + 1U];
						m_buffer[i * 2U + 1U] = m_buffer32[i * 2U + 0U];
					}
					break;
			}
			break;
	}

	m_callback->callback(m_buffer, n, m_id);

	return true;
}

void CSoundFileReader::rewind()
{
	wxASSERT(m_file != NULL);

	m_file->Seek(m_offset);

	m_read = 0U;
}

#endif
