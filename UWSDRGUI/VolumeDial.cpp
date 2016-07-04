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

#include "VolumeDial.h"

#include "UWSDRDefs.h"

#include <cmath>


BEGIN_EVENT_TABLE(CVolumeDial, wxPanel)
	EVT_PAINT(CVolumeDial::onPaint)
	EVT_MOTION(CVolumeDial::onMouse)
	EVT_LEFT_DOWN(CVolumeDial::onMouse)
END_EVENT_TABLE()

CVolumeDial::CVolumeDial(wxWindow* parent, int id, int min, int max, int value, IDialCallback* callback, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_width(size.GetWidth()),
m_height(size.GetHeight()),
m_callback(callback),
m_bitmap(NULL),
m_min(min),
m_max(max),
m_value(value)
{
	wxASSERT(m_max > m_min);
	wxASSERT(m_value >= m_min && value < m_max);
	wxASSERT(m_width == m_height);
	wxASSERT(m_callback != NULL);

	m_bitmap = new wxBitmap(m_width, m_height);

	drawDial();
}

CVolumeDial::~CVolumeDial()
{
	delete m_bitmap;
}

bool CVolumeDial::Enable(bool enable)
{
	bool ret = wxPanel::Enable(enable);

	drawDial();

	return ret;
}

void CVolumeDial::setValue(unsigned int value)
{
	m_value = value;

	drawDial();
}

void CVolumeDial::drawDial()
{
	wxMemoryDC dc;
	dc.SelectObject(*m_bitmap);

#if defined(__WXMSW__)
	int major, minor;
	::wxGetOsVersion(&major, &minor);

	wxColour bgColour;
	if (major >= 6)
		bgColour = wxColour(0xF0, 0xF0, 0xF0);		// Windows Vista and newer
	else if (major == 5 && minor >= 1)
		bgColour = wxColour(0xEC, 0xE9, 0xD8);		// Windows XP
	else
		bgColour = wxColour(0xD4, 0xD0, 0xC8);		// Windows 2000 and earlier
#elif defined(__WXGTK__)
	wxColour bgColour(0xF0, 0xF1, 0xF2);
	// wxColour bgColour(0xED, 0xE9, 0xE3);
#elif defined(__WXMAC__)
	wxColour bgColour(0xE8, 0xE8, 0xE8);
#else
#error "Unknown platform"
#endif

	wxBrush brush0(bgColour);
	wxPen pen0(bgColour);

	dc.SetPen(pen0);
	dc.SetBrush(brush0);
	dc.DrawRectangle(0, 0, m_width, m_height);

	if (IsEnabled()) {
		dc.SetBrush(*wxLIGHT_GREY_BRUSH);
		dc.SetPen(wxPen(*wxBLACK, 2, wxSOLID));
	} else {
		dc.SetBrush(brush0);
		dc.SetPen(wxPen(*wxLIGHT_GREY, 2, wxSOLID));
	}

	dc.DrawEllipse(1, 1, m_width - 2, m_height - 2);

	double incr = 270.0 / double(m_max - m_min); 

	double angle = (double(m_value - m_min) * incr + 45.0) * (M_PI / 180.0);

	double xFrac = ::sin(angle);
	double yFrac = ::cos(angle);

	int startX = m_width / 2 - int(double(m_width / 2 - 10) * xFrac + 0.5);
	int startY = m_height / 2 + int(double(m_height / 2 - 10) * yFrac + 0.5);

	int endX = m_width / 2 - int(double(m_width / 2 - 20) * xFrac + 0.5);
	int endY = m_height / 2 + int(double(m_height / 2 - 20) * yFrac + 0.5);

	if (IsEnabled())
		dc.SetPen(wxPen(*wxBLACK, 5, wxSOLID));
	else
		dc.SetPen(wxPen(*wxLIGHT_GREY, 2, wxSOLID));

	dc.DrawLine(startX, startY, endX, endY);	

	dc.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);
}

void CVolumeDial::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	show(dc);
}

void CVolumeDial::show(wxDC& dc)
{
	dc.DrawBitmap(*m_bitmap, 0, 0, false);
}

void CVolumeDial::onMouse(wxMouseEvent& event)
{
	if (!event.LeftIsDown())
		return;

	long diffX = event.GetX() - m_width / 2;
	long diffY = m_height / 2 - event.GetY();
	int   dist = int(::sqrt(double(diffX * diffX + diffY * diffY)) + 0.5);

	if (dist > (m_width - 2) / 2) {
		if (event.LeftDown())
			event.Skip();

		return;
	}

	double angle = 180.0 * ::atan2(double(diffY), double(-diffX)) / M_PI;

	if (angle < -90.0)
		angle += 450.0;
	else
		angle += 90.0;

	if (angle < 45.0 || angle > 315.0) {
		if (event.LeftDown())
			event.Skip();

		return;
	}

	angle -= 45.0;

	int value = m_min + int((angle / 270.0) * double(m_max - m_min));

	if (value != m_value) {
		m_value = value;

		m_callback->dialMoved(GetId(), m_value);

		drawDial();
	}

	if (event.LeftDown())
		event.Skip();
}
