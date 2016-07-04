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

#ifndef	FreqDisplay_H
#define	FreqDisplay_H

#include <wx/wx.h>

#include "Frequency.h"

class CFreqDisplay : public wxPanel {
public:
	CFreqDisplay(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style = 0L, const wxString& name = wxPanelNameStr);
	virtual ~CFreqDisplay();

	virtual void setMaxFrequency(const CFrequency& frequency);

	virtual void setFrequency(const CFrequency& frequency);

	void onPaint(wxPaintEvent& event);

private:
	int        m_width;
	int        m_height;
	wxBitmap*  m_bitmap;
	CFrequency m_lastFrequency;
	wxColour   m_lightColour;
	int        m_mhzDigits;

	DECLARE_EVENT_TABLE()

	void drawDigit(wxDC& dc, int width, int height, int thickness, int x, int y, unsigned int n, bool dot);

	void show(wxDC& dc);
	void clearGraph();
};

#endif
