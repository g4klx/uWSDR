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

#include "RingBuffer.h"


CRingBuffer::CRingBuffer(unsigned int length, unsigned int step) :
m_length(length),
m_step(step),
m_buffer(NULL),
m_iPtr(0U),
m_oPtr(0U),
m_state(RBSTATE_EMPTY)
{
	wxASSERT(length > 0U);
	wxASSERT(step > 0U);

	m_buffer = new float[length * step];

	::memset(m_buffer, 0x00, length * step * sizeof(float));
}

CRingBuffer::~CRingBuffer()
{
	delete[] m_buffer;
}

#if defined(__WXDEBUG__)
void CRingBuffer::dump(const wxString& title) const
{
	wxLogMessage(title);
	wxLogMessage(wxT("Length: 0x%05X  Step: %u  oPtr: 0x%05X  iPtr: 0x%05X  State: %d"), m_length, m_step, m_oPtr / m_step, m_iPtr / m_step, m_state);

	wxLogMessage(wxT(":"));

	unsigned int n = 0U;
	for (unsigned int i = 0U; i < m_length; i += 16) {
		wxString text;
		text.Printf(wxT("%05X:  "), i);

		for (unsigned int j = 0U; j < 16; j++) {
			for (unsigned int k = 0U; k < m_step; k++) {
				if (k > 0U)
					text.Append(wxT(":"));

				wxString buf;
				buf.Printf(wxT("%f"), m_buffer[n++]);
				text.Append(buf);
			}

			if ((i + j) >= m_length)
				break;

			text.Append(wxT("  "));
		}

		wxLogMessage(text);
	}
}
#endif
