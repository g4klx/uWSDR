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

#include "Frequency.h"

CFrequency::CFrequency(wxInt64 hz) :
m_hz(hz)
{
}

CFrequency::CFrequency(const CFrequency& frequency) :
m_hz(frequency.m_hz)
{
}

CFrequency::CFrequency(const wxString& freq) :
m_hz(0LL)
{
	set(freq);
}

CFrequency::CFrequency() :
m_hz(0LL)
{
}

CFrequency::~CFrequency()
{
}

void CFrequency::add(wxInt64 hz)
{
	m_hz += hz;
}

void CFrequency::sub(wxInt64 hz)
{
	m_hz -= hz;
}

void CFrequency::set(wxInt64 hz)
{
	m_hz = hz;
}

bool CFrequency::set(const wxString& freq)
{
	if (freq.Freq(wxT('.')) > 1)
		return false;

	int n = freq.length();

	bool error = false;
	for (int i = 0; i < n; i++) {
		wxChar c = freq.GetChar(i);

		switch (c) {
			case wxT('0'): case wxT('1'): case wxT('2'): case wxT('3'):
			case wxT('4'): case wxT('5'): case wxT('6'): case wxT('7'):
			case wxT('8'): case wxT('9'): case wxT('.'):
				break;
			default:
				error = true;
				break;
		}
	}

	if (error)
		return false;

	int pos = freq.Find(wxT('.'));

	if (pos == -1) {
		long temp;
		freq.ToLong(&temp);
		m_hz = temp * 1000000LL;
	} else {
		wxString hertz = freq.Mid(pos + 1);

		hertz.Append(wxT("00000000"));
		hertz.Truncate(8);				// Maybe wrong
		hertz.insert(6, wxT("."));

		long temp;

		// Parse the MHz
		freq.Left(pos).ToLong(&temp);
		m_hz = temp * 1000000LL;

		// Parse the Hz
		hertz.ToLong(&temp);
		m_hz += temp;
	}

	return true;
}


wxString CFrequency::getString(unsigned int decimals) const
{
	wxString text;

	if (decimals > 0U) {
		wxString hertz;

		hertz.Printf(wxT("%06lld"), m_hz % 1000000LL);
		hertz.Truncate(decimals);

		text.Printf(wxT("%lld.%s"), m_hz / 1000000LL, hertz.c_str());
	} else {
		text.Printf(wxT("%lld"), m_hz / 1000000LL);
	}

	return text;
}

wxInt64 CFrequency::get() const
{
	return m_hz;
}
