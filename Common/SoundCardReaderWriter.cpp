/*
 *   Copyright (C) 2006-2007,2013 by Jonathan Naylor G4KLX
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

#include "SoundCardReaderWriter.h"

#if defined(USE_PORTAUDIO)

int scrwCallback(const void* input, void* output, unsigned long nSamples, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	wxASSERT(userData != NULL);

	CSoundCardReaderWriter* object = reinterpret_cast<CSoundCardReaderWriter*>(userData);

	return object->callback(input, output, nSamples, timeInfo, statusFlags);
}


CSoundCardReaderWriter::CSoundCardReaderWriter(int inDev, int outDev, unsigned int inChannels, unsigned int outChannels) :
m_inDev(inDev),
m_outDev(outDev),
m_inChannels(inChannels),
m_outChannels(outChannels),
m_blockSize(0U),
m_callback(NULL),
m_id(0),
m_stream(NULL),
m_buffer(NULL),
m_lastBuffer(NULL),
m_inBuffer(NULL),
m_outBuffer(NULL),
m_requests(0),
m_underruns(0),
m_overruns(0),
m_enabled(false),
m_opened(0),
m_active(false)
{
	wxASSERT(inChannels <= 2U);
	wxASSERT(outChannels <= 2U);
	wxASSERT(inChannels > 0U || outChannels > 0U);
}

CSoundCardReaderWriter::~CSoundCardReaderWriter()
{
	delete   m_buffer;
	delete[] m_lastBuffer;
	delete[] m_inBuffer;
	delete[] m_outBuffer;
}

void CSoundCardReaderWriter::setCallback(IDataCallback* callback, int id)
{
	wxASSERT(callback != NULL);

	m_callback = callback;
	m_id       = id;
}

bool CSoundCardReaderWriter::open(float sampleRate, unsigned int blockSize)
{
	if (m_opened > 0) {
		m_opened++;
		return true;
	}

	m_blockSize = blockSize;
	m_enabled   = false;

	PaError error = ::Pa_Initialize();
	if (error != paNoError) {
		writeLog(wxT("Pa_Initialise()"), error);
		return false;
	}

	PaStreamParameters* pParamsIn  = NULL;
	PaStreamParameters* pParamsOut = NULL;

	PaStreamParameters paramsIn;
	PaStreamParameters paramsOut;

	if (m_inDev != -1 && m_inChannels > 0U) {
		m_inBuffer  = new float[blockSize * 2U];

		const PaDeviceInfo* inInfo  = ::Pa_GetDeviceInfo(m_inDev);
		if (inInfo == NULL) {
			wxLogError(wxT("SoundCardReaderWriter: received NULL from Pa_GetDeviceInfo()"));
			return false;
		}

		paramsIn.device                    = m_inDev;
		paramsIn.channelCount              = m_inChannels;
		paramsIn.sampleFormat              = paFloat32;
		paramsIn.hostApiSpecificStreamInfo = NULL;
		paramsIn.suggestedLatency          = inInfo->defaultLowInputLatency;

		pParamsIn = &paramsIn;
	}

	if (m_outDev != -1 && m_outChannels > 0U) {
		m_outBuffer = new float[blockSize * 2U];
		m_buffer    = new CRingBuffer(blockSize * 5U, 2U);

		m_lastBuffer = new float[blockSize * 2U];
		::memset(m_lastBuffer, 0x00, blockSize * 2U * sizeof(float));

		const PaDeviceInfo* outInfo = ::Pa_GetDeviceInfo(m_outDev);
		if (outInfo == NULL) {
			wxLogError(wxT("SoundCardReaderWriter: received NULL from Pa_GetDeviceInfo()"));
			return false;
		}

		paramsOut.device                    = m_outDev;
		paramsOut.channelCount              = m_outChannels;
		paramsOut.sampleFormat              = paFloat32;
		paramsOut.hostApiSpecificStreamInfo = NULL;
		paramsOut.suggestedLatency          = outInfo->defaultLowOutputLatency;

		pParamsOut = &paramsOut;
	}

	error = ::Pa_OpenStream(&m_stream, pParamsIn, pParamsOut, sampleRate, blockSize, paNoFlag, &scrwCallback, this);
	if (error != paNoError) {
		::Pa_Terminate();
		writeLog(wxT("Pa_OpenStream()"), error);
		return false;
	}

	error = ::Pa_StartStream(m_stream);
	if (error != paNoError) {
		::Pa_CloseStream(m_stream);
		m_stream = NULL;

		::Pa_Terminate();
		writeLog(wxT("Pa_StartStream()"), error);
		return false;
	}

	m_opened++;
	m_active = true;

	wxLogMessage(wxT("SoundCardReaderWriter: started with input device %d and output device %d"), m_inDev, m_outDev);

	return true;
}

void CSoundCardReaderWriter::write(const float* buffer, unsigned int nSamples)
{
	if (!m_enabled || m_outDev == -1 || m_outChannels == 0U)
		return;

	if (nSamples == 0U)
		return;

	wxASSERT(buffer != NULL);

	unsigned int n = m_buffer->addData(buffer, nSamples);

	if (n != nSamples)
		m_overruns++;
}

int CSoundCardReaderWriter::callback(const void* input, void* output, unsigned long nSamples, const PaStreamCallbackTimeInfo* WXUNUSED(timeInfo), PaStreamCallbackFlags WXUNUSED(statusFlags))
{
	m_requests++;

	if (m_inDev != -1) {
		wxASSERT(input != NULL);
		wxASSERT(m_callback != NULL);
		wxASSERT(m_inBuffer != NULL);

		float* in  = (float*)input;

		if (m_inChannels == 1U) {
			for (unsigned int i = 0U; i < nSamples; i++) {
				m_inBuffer[i * 2U + 0U] = in[i];
				m_inBuffer[i * 2U + 1U] = in[i];
			}

			in = m_inBuffer;
		}

		m_callback->callback(in, nSamples, m_id);
	}

	if (m_outDev != -1) {
		wxASSERT(output != NULL);
		wxASSERT(m_buffer != NULL);
		wxASSERT(m_outBuffer != NULL);
		wxASSERT(m_lastBuffer != NULL);

		float* out = (float*)output;

		if (m_buffer->dataSpace() >= nSamples) {
			m_buffer->getData(m_outBuffer, nSamples);
		} else {
			::memcpy(m_outBuffer, m_lastBuffer, nSamples * 2U * sizeof(float));
			m_underruns++;
		}

		::memcpy(m_lastBuffer, m_outBuffer, nSamples * 2U * sizeof(float));

		switch (m_outChannels) {
			case 1U: {
					for (unsigned int i = 0U; i < nSamples; i++)
						out[i] = m_outBuffer[i * 2U + 0U];
				}
				break;
			case 2U:
				::memcpy(out, m_outBuffer, nSamples * 2U * sizeof(float));
				break;
		}
	}

	return paContinue;
}

/*
 * Close the sound card on the first call to close() all later ones are NOPs until the last one which also
 * delete's the object.
 */
void CSoundCardReaderWriter::close()
{
	if (!m_active) {
		if (m_opened == 0U) {
			delete this;
			return;
		}

		m_opened--;
		return;
	}

	m_opened--;
	m_active = false;

	PaError error = ::Pa_AbortStream(m_stream);
	if (error != paNoError)
		writeLog(wxT("Pa_AbortStream()"), error);

	error = ::Pa_CloseStream(m_stream);
	if (error != paNoError)
		writeLog(wxT("Pa_CloseStream()"), error);

	error = ::Pa_Terminate();
	if (error != paNoError)
		writeLog(wxT("Pa_Terminate()"), error);

	wxLogMessage(wxT("SoundCardReaderWriter: %u underruns and %u overruns from %u requests"), m_underruns, m_overruns, m_requests);
}

void CSoundCardReaderWriter::enable(bool enable)
{
	if (m_outDev == -1 || m_outChannels == 0U)
		return;

	m_enabled = enable;

	if (!enable)
		::memset(m_lastBuffer, 0x00, m_blockSize * 2U * sizeof(float));
}

void CSoundCardReaderWriter::disable()
{
	enable(false);
}

void CSoundCardReaderWriter::purge()
{
}

bool CSoundCardReaderWriter::hasClock()
{
	return true;
}

void CSoundCardReaderWriter::clock()
{
}

void CSoundCardReaderWriter::writeLog(const wxString& function, PaError error) const
{
	wxString message(::Pa_GetErrorText(error), wxConvLocal);

	wxLogError(wxT("SoundCardReaderWriter: received %d:%s from %s"), error, message.c_str(), function.c_str());
}

#else

CSoundCardReaderWriter::CSoundCardReaderWriter(int WXUNUSED(inDev), int WXUNUSED(outDev), unsigned int WXUNUSED(inChannels), unsigned int WXUNUSED(outChannels))
{
}

CSoundCardReaderWriter::~CSoundCardReaderWriter()
{
}

void CSoundCardReaderWriter::setCallback(IDataCallback* WXUNUSED(callback), int WXUNUSED(id))
{
}

bool CSoundCardReaderWriter::open(float WXUNUSED(sampleRate), unsigned int WXUNUSED(blockSize))
{
	wxLogError(wxT("SoundCardReaderWriter: UWSDR has been built without PortAudio support"));

	return false;
}

void CSoundCardReaderWriter::write(const float* WXUNUSED(buffer), unsigned int WXUNUSED(nSamples))
{
}

void CSoundCardReaderWriter::close()
{
}

void CSoundCardReaderWriter::enable(bool WXUNUSED(enable))
{
}

void CSoundCardReaderWriter::disable()
{
}

void CSoundCardReaderWriter::purge()
{
}

bool CSoundCardReaderWriter::hasClock()
{
	return false;
}

void CSoundCardReaderWriter::clock()
{
}

#endif
