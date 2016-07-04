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

#include "FreqDial.h"

#include "UWSDRDefs.h"

#include <cmath>

enum {
	MENU_1X = 6743,
	MENU_4X,
	MENU_9X,
	MENU_16X,
	MENU_25X
};

BEGIN_EVENT_TABLE(CFreqDial, wxPanel)
	EVT_PAINT(CFreqDial::onPaint)
	EVT_LEFT_DOWN(CFreqDial::onMouse)
	EVT_MOTION(CFreqDial::onMouse)
	EVT_RIGHT_DOWN(CFreqDial::onMouseMenu)
	EVT_MENU(MENU_1X,  CFreqDial::onMenu)
	EVT_MENU(MENU_4X,  CFreqDial::onMenu)
	EVT_MENU(MENU_9X,  CFreqDial::onMenu)
	EVT_MENU(MENU_16X, CFreqDial::onMenu)
	EVT_MENU(MENU_25X, CFreqDial::onMenu)
END_EVENT_TABLE()

CFreqDial::CFreqDial(wxWindow* parent, int id, IDialCallback* callback, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_menu(NULL),
m_width(size.GetWidth()),
m_height(size.GetHeight()),
m_callback(callback),
m_bitmap(NULL),
m_angle(0.0),
m_mult(9)
{
	wxASSERT(m_height == m_width);
	wxASSERT(m_callback != NULL);

	m_bitmap = new wxBitmap(m_width, m_height);

	m_menu = new wxMenu();
	m_menu->AppendRadioItem(MENU_1X,  _("1x"));
	m_menu->AppendRadioItem(MENU_4X,  _("4x"));
	m_menu->AppendRadioItem(MENU_9X,  _("9x"));
	m_menu->AppendRadioItem(MENU_16X, _("16x"));
	m_menu->AppendRadioItem(MENU_25X, _("25x"));

	drawDial();
}

CFreqDial::~CFreqDial()
{
	delete m_menu;
	delete m_bitmap;
}

void CFreqDial::drawDial()
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
	wxColour bgColour(0xF0, 0xF0, 0xF0);
#else
#error "Unknown platform"
#endif

	wxBrush brush0(bgColour);
	wxPen pen0(bgColour);

	dc.SetPen(pen0);
	dc.SetBrush(brush0);
	dc.DrawRectangle(0, 0, m_width, m_height);

	int middleX = m_width / 2;
	int middleY = m_height / 2;

	dc.SetBrush(*wxLIGHT_GREY_BRUSH);
	wxPen pen1(*wxBLACK, 2, wxSOLID);
	dc.SetPen(pen1);
	dc.DrawCircle(middleX, middleY, (m_width - 2) / 2);

	int x = m_width / 2 - int(double(m_width / 2 - 25) * ::sin(m_angle * (M_PI / 180.0)) + 0.5);
	int y = m_height / 2 + int(double(m_height / 2 - 25) * ::cos(m_angle * (M_PI / 180.0)) + 0.5);

	dc.SetBrush(*wxBLACK_BRUSH);
	wxPen pen2(*wxWHITE, 2, wxSOLID);
	dc.SetPen(pen2);
	dc.DrawCircle(x, y, 20);

	dc.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);
}

void CFreqDial::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	show(dc);
}

void CFreqDial::show(wxDC& dc)
{
	dc.DrawBitmap(*m_bitmap, 0, 0, false);
}

void CFreqDial::onMouseMenu(wxMouseEvent& event)
{
	switch (m_mult) {
		case 1:
			m_menu->Check(MENU_1X, true);
			break;
		case 4:
			m_menu->Check(MENU_4X, true);
			break;
		case 9:
			m_menu->Check(MENU_9X, true);
			break;
		case 16:
			m_menu->Check(MENU_16X, true);
			break;
		case 25:
			m_menu->Check(MENU_25X, true);
			break;
		default:
			wxLogError(wxT("Unknown freq dial multiplier = %u"), m_mult);
			break;
	}

	int x = event.GetX();
	int y = event.GetY();

	PopupMenu(m_menu, x, y);
}

void CFreqDial::onMouse(wxMouseEvent& event)
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

	if (angle != m_angle) {
		int value = (angle > m_angle) ? 1 : -1;

		m_angle = angle;

		m_callback->dialMoved(GetId(), value * m_mult);

		drawDial();
	}

	if (event.LeftDown())
		event.Skip();
}

void CFreqDial::onMenu(wxCommandEvent& event)
{
	int id = event.GetId();

	switch (id) {
		case MENU_25X:
			m_mult = 25;
			break;
		case MENU_16X:
			m_mult = 16;
			break;
		case MENU_9X:
			m_mult = 9;
			break;
		case MENU_4X:
			m_mult = 4;
			break;
		case MENU_1X:
			m_mult = 1;
			break;
		default:
			wxLogError(wxT("Unknown freq diql menu option = %d"), id);
			break;
	}
}
