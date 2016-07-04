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

#ifndef	FrequencyRangeArray_H
#define	FrequencyRangeArray_H

#include "Frequency.h"

#include <vector>

#include <wx/wx.h>

class CFrequencyRange {
public:
	CFrequencyRange(const wxString& start, const wxString& stop);
	CFrequencyRange(const CFrequencyRange& range);
	~CFrequencyRange();

	bool inRange(const CFrequency& freq) const;

	CFrequencyRange& operator=(const CFrequencyRange& range);

private:
	CFrequency m_start;
	CFrequency m_stop;
};

class CFrequencyRangeArray {
public:
	CFrequencyRangeArray(const CFrequencyRangeArray& range);
	CFrequencyRangeArray();
	~CFrequencyRangeArray();

	void addRange(const wxString& start, const wxString& stop);

	bool inRange(const CFrequency& freq) const;

	bool isEmpty() const;

	unsigned int getCount() const;

	CFrequencyRangeArray& operator=(const CFrequencyRangeArray& range);

private:
	std::vector<CFrequencyRange> m_range;
};

#endif
