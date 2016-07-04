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

#include "ExternalProtocolHandler.h"

const unsigned int NET_BUFFER_LEN   = 2000U;
const unsigned int AUDIO_BUFFER_LEN = 5000U;

const unsigned int EXT_SAMPLE_RATE = 8000U;

const unsigned int RAW_BLOCK_SIZE  = 128U;

const unsigned int MAX_SILENCE_FRAMES = 5U;

const unsigned int MAX_MISSING_FRAMES = 20U;

CExternalProtocolHandler::CExternalProtocolHandler(float sampleRate, unsigned int blockSize, const wxString& name, EXTERNALADDRS addrs, IExternalControlInterface* iface) :
m_sampleRate(sampleRate),
m_blockSize(blockSize),
m_skipFactor(0U),
m_upsampler(NULL),
m_control(iface),
m_socket(NULL),
m_addrs(addrs),
m_callback(NULL),
m_id(0),
m_connected(CONNECTTYPE_NONE),
m_transmit(false),
m_netBuffer(NULL),
m_audioBuffer(NULL),
m_remoteAddress(),
m_remotePort(0U),
m_pingInTimeout(0U),
m_pingOutTimeout(0U),
m_pingInTimer(0U),
m_pingOutTimer(0U),
m_outSerial(0U),
m_inSerial(0U)
{
	wxASSERT(iface != NULL);

	m_skipFactor = (unsigned int)sampleRate / EXT_SAMPLE_RATE;

	m_upsampler = new CUpsampler(float(EXT_SAMPLE_RATE), sampleRate);

	m_pingInTimeout  = 30U * (unsigned int)(sampleRate / blockSize);
	m_pingOutTimeout = 10U * (unsigned int)(sampleRate / blockSize);

	unsigned int port;
	if (name.IsSameAs(wxT("SDR 1")))
		port = 8062U;
	else if (name.IsSameAs(wxT("SDR 2")))
		port = 8063U;
	else if (name.IsSameAs(wxT("SDR 3")))
		port = 8063U;
	else if (name.IsSameAs(wxT("SDR 4")))
		port = 8064U;
	else
		return;

	wxString address;
	if (addrs == EXTERNALADDRS_HOST) {
		address = wxT("127.0.0.1");
		wxLogMessage(wxT("Listening on 127.0.0.1:%u"), port);
	} else {
		wxLogMessage(wxT("Listening on *.*.*.*:%u"), port);
	}

	m_socket = new CUDPReaderWriter(address, port);

	m_netBuffer   = new unsigned char[NET_BUFFER_LEN];
	m_audioBuffer = new float[AUDIO_BUFFER_LEN];
}

CExternalProtocolHandler::~CExternalProtocolHandler()
{
	delete m_socket;
	delete m_upsampler;

	delete[] m_netBuffer;
	delete[] m_audioBuffer;
}

bool CExternalProtocolHandler::open()
{
	return m_socket->open();
}

void CExternalProtocolHandler::close()
{
	if (m_transmit)
		m_control->setExtTransmit(false);

	m_connected = CONNECTTYPE_NONE;

	m_socket->close();
}

void CExternalProtocolHandler::setCallback(IDataCallback* callback, int id)
{
	wxASSERT(callback != NULL);

	m_callback = callback;
	m_id       = id;
}

void CExternalProtocolHandler::clock()
{
	if (m_connected != CONNECTTYPE_NONE) {
		// Let our partner know that we're still alive
		m_pingOutTimer++;
		if (m_pingOutTimer >= m_pingOutTimeout) {
			m_socket->write((unsigned char*)"QRZ\n", 4U, m_remoteAddress, m_remotePort);
			m_pingOutTimer = 0U;
		}

		// Has our partner gone away?
		m_pingInTimer++;
		if (m_pingInTimer >= m_pingInTimeout) {
			if (m_transmit)
				m_control->setExtTransmit(false);
			m_connected = CONNECTTYPE_NONE;
		}
	}

	in_addr address;
	unsigned int port;
	int n = m_socket->read(m_netBuffer, NET_BUFFER_LEN, address, port);
	if (n <= 0)
		return;

	if (m_connected != CONNECTTYPE_NONE) {
		// Ensure that the incoming packet matches our partner
		if (m_remoteAddress.s_addr != address.s_addr || m_remotePort != port)
			return;

		if (m_netBuffer[0U] == 0xFFU) {
			if (m_transmit && m_connected == CONNECTTYPE_AUDIO) {
				m_pingInTimer = 0U;

				// Normalise for wrapped incoming serial numbers
				unsigned int newSerial = m_netBuffer[1U];
				if (newSerial < m_inSerial)
					newSerial += 255U;

				// Check for an out of order packet
				if ((newSerial - m_inSerial) >= MAX_MISSING_FRAMES)
					return;

				unsigned int audioSize = (n - 4U) / sizeof(wxFloat32);

				// Fill in gaps to the maximum
				::memset(m_audioBuffer, 0x00U, audioSize * m_skipFactor * 2U * sizeof(wxFloat32));

				unsigned int count = 0U;
				for (unsigned char serial = m_inSerial; count < MAX_SILENCE_FRAMES && serial != m_netBuffer[1U]; serial++, count++)
					m_callback->callback(m_audioBuffer, audioSize * m_skipFactor, m_id);

				const wxFloat32* p1 = (wxFloat32*)(m_netBuffer + 4U);

				m_upsampler->process(p1, audioSize, m_audioBuffer);

				// Shuffle it all down
				float* p2 = m_audioBuffer + (audioSize * m_skipFactor) - 1U;
				float* p3 = m_audioBuffer + (audioSize * m_skipFactor * 2U) - 1U;

				for (unsigned int i = 0U; i < (audioSize * m_skipFactor); i++) {
					*p3-- = *p2;	// L
					*p3-- = *p2;	// R
					p2--;
				}

				m_callback->callback(m_audioBuffer, audioSize * m_skipFactor, m_id);

				m_inSerial = m_netBuffer[1U] + 1U;
			}
		} else if (m_netBuffer[0U] == 0xFEU) {
			// Quietly ignore
		} else if (::memcmp(m_netBuffer, "QRZ\n", 4U) == 0) {
			m_pingInTimer = 0U;
		} else if (::memcmp(m_netBuffer, "FREQ ", 5U) == 0) {
			if (m_transmit) {
				m_socket->write((unsigned char*)"NAK Transmitting\n", 17U, address, port);
				return;
			}

			unsigned long freqHz = ::atol((char*)(m_netBuffer + 5U));
			CFrequency freq(freqHz);
			bool ret = m_control->setExtFrequency(freq);
			if (!ret) {
				m_socket->write((unsigned char*)"NAK Out of range\n", 17U, address, port);
				return;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "MODE USB\n", 9U) == 0) {
			if (m_transmit) {
				m_socket->write((unsigned char*)"NAK Transmitting\n", 17U, address, port);
				return;
			}

			bool ret = m_control->setExtMode(MODE_USB);
			if (!ret) {
				m_socket->write((unsigned char*)"NAK Not allowed\n", 16U, address, port);
				return;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "MODE LSB\n", 9U) == 0) {
			if (m_transmit) {
				m_socket->write((unsigned char*)"NAK Transmitting\n", 17U, address, port);
				return;
			}

			bool ret = m_control->setExtMode(MODE_LSB);
			if (!ret) {
				m_socket->write((unsigned char*)"NAK Not allowed\n", 16U, address, port);
				return;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "MODE CW\n", 8U) == 0) {
			if (m_transmit) {
				m_socket->write((unsigned char*)"NAK Transmitting\n", 17U, address, port);
				return;
			}

			bool ret = m_control->setExtMode(MODE_CWUN);
			if (!ret) {
				m_socket->write((unsigned char*)"NAK Not allowed\n", 16U, address, port);
				return;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "MODE FM\n", 8U) == 0) {
			if (m_transmit) {
				m_socket->write((unsigned char*)"NAK Transmitting\n", 17U, address, port);
				return;
			}

			bool ret = m_control->setExtMode(MODE_FMN);
			if (!ret) {
				m_socket->write((unsigned char*)"NAK Not allowed\n", 16U, address, port);
				return;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "MODE AM\n", 8U) == 0) {
			if (m_transmit) {
				m_socket->write((unsigned char*)"NAK Transmitting\n", 17U, address, port);
				return;
			}

			bool ret = m_control->setExtMode(MODE_AM);
			if (!ret) {
				m_socket->write((unsigned char*)"NAK Not allowed\n", 16U, address, port);
				return;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "PTT True\n", 9U) == 0) {
			if (m_connected != CONNECTTYPE_AUDIO) {
				m_socket->write((unsigned char*)"NAK Wrong mode\n", 15U, address, port);
				return;
			}

			if (!m_transmit) {
				bool ret = m_control->setExtTransmit(true);
				if (!ret) {
					m_socket->write((unsigned char*)"NAK Not allowed\n", 16U, address, port);
					return;
				}

				m_transmit = true;
				m_inSerial = 0U;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "PTT False\n", 10U) == 0) {
			if (m_connected != CONNECTTYPE_AUDIO) {
				m_socket->write((unsigned char*)"NAK Wrong mode\n", 15U, address, port);
				return;
			}

			if (m_transmit) {
				m_control->setExtTransmit(false);
				m_transmit = false;
			}

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "STOP\n", 5U) == 0) {
			if (m_transmit)
				m_control->setExtTransmit(false);

			m_connected = CONNECTTYPE_NONE;

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);
		} else if (::memcmp(m_netBuffer, "START", 5U) == 0) {
			// Quietly ignore
		} else {
			m_socket->write((unsigned char*)"NAK Invalid command\n", 20U, address, port);
		}
	} else {
		// Validate the source address, in the case of EXTERNADDRS_HOST we're only listening on the local host anyway
		if (m_addrs == EXTERNALADDRS_LOCAL) {
			if ((address.s_addr & 0xFF000000U) != 0x7F000000U &&	// 127.0.0.0/8
			    (address.s_addr & 0xFF000000U) != 0x0A000000U &&	// 10.0.0.0/8
			    (address.s_addr & 0xFFF00000U) != 0xAC100000U &&	// 172.16.x.x/12
				(address.s_addr & 0xFFFF0000U) != 0xC0A80000U) {	// 192.168.0.0/16
				wxLogMessage(wxT("Ignored connect from invalid address: %d.%d.%d.%d"),
					(address.s_addr >> 24) & 0xFFU,
					(address.s_addr >> 16) & 0xFFU,
					(address.s_addr >> 8)  & 0xFFU,
					(address.s_addr >> 0)  & 0xFFU);
				return;
			}
		}

		if (::memcmp(m_netBuffer, "START AUDIO\n", 12U) == 0) {
			m_inSerial  = 0U;
			m_outSerial = 0U;

			m_connected = CONNECTTYPE_AUDIO;
			m_transmit  = false;

			m_pingInTimer  = 0U;
			m_pingOutTimer = 0U;

			m_remoteAddress = address;
			m_remotePort    = port;

			m_socket->write((unsigned char*)"ACK 8000 F32 I\n", 15U, address, port);

			wxLogMessage(wxT("Audio connect from address: %d.%d.%d.%d"),
				(address.s_addr >> 24) & 0xFFU,
				(address.s_addr >> 16) & 0xFFU,
				(address.s_addr >> 8)  & 0xFFU,
				(address.s_addr >> 0)  & 0xFFU);
		} else if (::memcmp(m_netBuffer, "START RAW\n", 10U) == 0) {
			m_inSerial  = 0U;
			m_outSerial = 0U;

			m_connected = CONNECTTYPE_RAW;
			m_transmit  = false;

			m_pingInTimer  = 0U;
			m_pingOutTimer = 0U;

			m_remoteAddress = address;
			m_remotePort    = port;

			char reply[25U];
			::sprintf(reply, "ACK %.0f F32 IQ\n", m_sampleRate);

			m_socket->write((unsigned char*)reply, ::strlen(reply), address, port);

			wxLogMessage(wxT("Raw connect from address: %d.%d.%d.%d"),
				(address.s_addr >> 24) & 0xFFU,
				(address.s_addr >> 16) & 0xFFU,
				(address.s_addr >> 8)  & 0xFFU,
				(address.s_addr >> 0)  & 0xFFU);
		} else if (::memcmp(m_netBuffer, "START\n", 6U) == 0) {
			m_connected = CONNECTTYPE_CONTROL;
			m_transmit  = false;

			m_pingInTimer  = 0U;
			m_pingOutTimer = 0U;

			m_remoteAddress = address;
			m_remotePort    = port;

			m_socket->write((unsigned char*)"ACK\n", 4U, address, port);

			wxLogMessage(wxT("Control connect from address: %d.%d.%d.%d"),
				(address.s_addr >> 24) & 0xFFU,
				(address.s_addr >> 16) & 0xFFU,
				(address.s_addr >> 8)  & 0xFFU,
				(address.s_addr >> 0)  & 0xFFU);
		} else {
			wxString text((char*)m_netBuffer, wxConvLocal, n);

			wxLogMessage(wxT("Invalid connect \"%s\" from address: %d.%d.%d.%d"), text.c_str(),
				(address.s_addr >> 24) & 0xFFU,
				(address.s_addr >> 16) & 0xFFU,
				(address.s_addr >> 8)  & 0xFFU,
				(address.s_addr >> 0)  & 0xFFU);
		}
	}
}

void CExternalProtocolHandler::writeAudio(const float* audio, unsigned int length)
{
	// We don't do full duplex
	if (m_connected != CONNECTTYPE_AUDIO || m_transmit)
		return;

	m_pingOutTimer = 0U;

	m_netBuffer[0U] = 0xFFU;
	m_netBuffer[1U] = m_outSerial;
	m_netBuffer[2U] = 0x00U;
	m_netBuffer[3U] = 0x00U;
	m_outSerial++;

	wxFloat32* p1 = (wxFloat32*)(m_netBuffer + 4U);
	const float* p2 = audio;

	// This downsamples, but since the audio is already band limited there are no aliasing issues - I think
	// Also go from stereo to mono
	length /= m_skipFactor * 2U;

	for (unsigned int i = 0U; i < length; i++, p1++, p2 += m_skipFactor * 2U)
		*p1 = *p2;

	m_socket->write(m_netBuffer, length * sizeof(wxFloat32) + 4U, m_remoteAddress, m_remotePort);
}

void CExternalProtocolHandler::writeRaw(const float* data, unsigned int length)
{
	if (m_connected != CONNECTTYPE_RAW)
		return;

	m_pingOutTimer = 0U;

	const float* p2 = data;

	for (unsigned int n = 0U; n < (m_blockSize / RAW_BLOCK_SIZE); n++) {
		m_netBuffer[0U] = 0xFEU;
		m_netBuffer[1U] = m_outSerial;
		m_netBuffer[2U] = 0x00U;
		m_netBuffer[3U] = 0x00U;
		m_outSerial++;

		wxFloat32* p1 = (wxFloat32*)(m_netBuffer + 4U);

		for (unsigned int i = 0U; i < RAW_BLOCK_SIZE; i++) {
			*p1++ = *p2++;		// I
			*p1++ = *p2++;		// Q
		}

		m_socket->write(m_netBuffer, length * sizeof(wxFloat32) + 4U, m_remoteAddress, m_remotePort);
	}
}
