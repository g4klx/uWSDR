/*
 *   Copyright (C) 2006-2008 by Jonathan Naylor G4KLX
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

#include "VoiceKeyer.h"

#include "UWSDRApp.h"


CVoiceKeyer::CVoiceKeyer() :
m_sampleRate(0.0F),
m_blockSize(0),
m_callback(NULL),
m_id(0),
m_status(VOICE_ABORT),
m_file(NULL)
{
}

CVoiceKeyer::~CVoiceKeyer()
{
}

bool CVoiceKeyer::open(float sampleRate, unsigned int blockSize)
{
	wxASSERT(sampleRate > 0.0F);

	m_sampleRate = sampleRate;
	m_blockSize  = blockSize;

	return true;
}

void CVoiceKeyer::close()
{
	if (m_file != NULL)
		m_file->close();

	delete this;
}

void CVoiceKeyer::purge()
{
	if (m_file != NULL)
		m_file->purge();
}

bool CVoiceKeyer::hasClock()
{
	return false;
}

void CVoiceKeyer::clock()
{
	if (m_status == VOICE_ABORT)
		return;

	if (m_file != NULL)
		m_file->clock();
}

void CVoiceKeyer::setCallback(IDataCallback* callback, int id)
{
	m_callback = callback;
	m_id       = id;
}

VOICEERROR CVoiceKeyer::send(const wxString& fileName, VOICESTATUS status)
{
	if (status == VOICE_ABORT && m_file == NULL)
		return VOICE_ERROR_NONE;

	if (status == VOICE_ABORT && m_file != NULL) {
		if (m_file != NULL) {
			m_file->close();
			m_file = NULL;
		}

		m_status = status;

		::wxGetApp().sendAudio(wxEmptyString, VOICE_TX_OFF);

		return VOICE_ERROR_NONE;
	}

	wxASSERT(status == VOICE_SINGLE || status == VOICE_CONTINUOUS);

	if (m_file == NULL) {
		m_file = new CSoundFileReader(fileName);

		bool ret = m_file->open(m_sampleRate, m_blockSize);
		if (!ret) {
			m_file = NULL;
			return VOICE_ERROR_FILE;
		}

		VOICEERROR err = ::wxGetApp().sendAudio(wxEmptyString, VOICE_TX_ON);
		if (err != VOICE_ERROR_NONE) {
			m_file->close();
			m_file = NULL;
			return err;
		}

		m_status = status;

		m_file->setCallback(this, 0);

		return VOICE_ERROR_NONE;
	}

	return VOICE_ERROR_TX;
}

void CVoiceKeyer::callback(float* buffer, unsigned int nSamples, int WXUNUSED(id))
{
	if (m_callback == NULL)
		return;

	// EOF in single mode means the end of transmission
	if (nSamples == 0 && m_status == VOICE_SINGLE) {
		nSamples = m_blockSize;
		::memset(buffer, 0x00, m_blockSize * 2 * sizeof(float));
		::wxGetApp().sendAudio(wxEmptyString, VOICE_ABORT);
	}

	// Restart the sound file and send a block of silence for now
	if (nSamples == 0 && m_status == VOICE_CONTINUOUS) {
		m_file->rewind();
		nSamples = m_blockSize;
		::memset(buffer, 0x00, m_blockSize * 2 * sizeof(float));
	}

	// Convert to our callback id
	m_callback->callback(buffer, nSamples, m_id);
}

bool CVoiceKeyer::isActive() const
{
	return m_status == VOICE_SINGLE || m_status == VOICE_CONTINUOUS;
}
