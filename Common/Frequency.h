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

#ifndef	Frequency_H
#define	Frequency_H

#include <wx/wx.h>

class CFrequency {
public:
	CFrequency(wxInt64 hz);
	CFrequency(const CFrequency& frequency);
	CFrequency(const wxString& freq);
	CFrequency();
	virtual ~CFrequency();

	virtual void add(wxInt64 hz);
	virtual void sub(wxInt64 hz);

	virtual void set(wxInt64 hz);
	virtual bool set(const wxString& freq);

	virtual wxString getString(unsigned int decimals = 8) const;
	virtual wxInt64  get() const;

	CFrequency operator+(wxInt64 hz)
	{
		CFrequency temp(*this);
		temp.add(hz);
		return temp;
	}

	CFrequency operator-(wxInt64 hz)
	{
		CFrequency temp(*this);
		temp.sub(hz);
		return temp;
	}

	CFrequency operator+(const CFrequency& freq)
	{
		CFrequency temp(*this);
		temp.add(freq.m_hz);
		return temp;
	}

	CFrequency operator-(const CFrequency& freq)
	{
		CFrequency temp(*this);
		temp.sub(freq.m_hz);
		return temp;
	}

	CFrequency& operator+=(wxInt64 hz)
	{
		this->add(hz);
		return *this;
	}

	CFrequency& operator-=(wxInt64 hz)
	{
		this->sub(hz);
		return *this;
	}		

	CFrequency& operator=(const CFrequency& frequency)
	{
		this->set(frequency.get());
		return *this;
	}

	bool operator==(const CFrequency& freq) const
	{
		return freq.m_hz == this->m_hz;
	}

	bool operator!=(const CFrequency& freq) const
	{
		return freq.m_hz != this->m_hz;
	}

	bool operator<(const CFrequency& freq) const
	{
		return freq.m_hz > this->m_hz;
	}

	bool operator<=(const CFrequency& freq) const
	{
		return freq.m_hz >= this->m_hz;
	}

	bool operator>(const CFrequency& freq) const
	{
		return freq.m_hz < this->m_hz;
	}

	bool operator>=(const CFrequency& freq) const
	{
		return freq.m_hz <= this->m_hz;
	}

private:
	wxInt64 m_hz;
};

#endif
