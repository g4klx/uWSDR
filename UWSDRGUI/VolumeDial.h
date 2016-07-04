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

#ifndef	VolumeDial_H
#define	VolumeDial_H

#include <wx/wx.h>

#include "DialCallback.h"

class CVolumeDial : public wxPanel {

    public:
	CVolumeDial(wxWindow* parent, int id, int min, int max, int value, IDialCallback* callback, const wxPoint& pos, const wxSize& size, long style = 0L, const wxString& name = wxPanelNameStr);
	virtual ~CVolumeDial();

	virtual bool Enable(bool enable = true);

	virtual void setValue(unsigned int value);

	void onPaint(wxPaintEvent& event);
	void onMouse(wxMouseEvent& event);

    private:
	int            m_width;
	int            m_height;
	IDialCallback* m_callback;
	wxBitmap*      m_bitmap;
	int            m_min;
	int            m_max;
	int            m_value;

	DECLARE_EVENT_TABLE()

	void drawDial();
	void show(wxDC& dc);
};

#endif
