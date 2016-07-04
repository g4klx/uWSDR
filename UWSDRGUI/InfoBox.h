/*
 *   Copyright (C) 2006-2007 by Jonathan Naylor G4KLX
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

#ifndef	InfoBox_H
#define	InfoBox_H

#include <wx/wx.h>

class CInfoBox : public wxPanel {

    public:
	CInfoBox(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style = 0L, const wxString& name = wxPanelNameStr);
	virtual ~CInfoBox();

	virtual void setVFO(int vfoNum);
	virtual void setTX(bool onOff);
	virtual void setRIT(bool onOff);
	virtual void setSplitShift(int splitShift);

	void onPaint(wxPaintEvent& event);

    private:
	int          m_width;
	int          m_height;
	wxBitmap*    m_bitmap;

	DECLARE_EVENT_TABLE()

	void show(wxDC& dc);
	void clearGraph();
};

#endif
