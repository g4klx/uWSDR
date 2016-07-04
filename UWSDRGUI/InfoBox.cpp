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

#include "InfoBox.h"

#include "UWSDRDefs.h"

const unsigned int FONT_SIZE = 10U;

BEGIN_EVENT_TABLE(CInfoBox, wxPanel)
	EVT_PAINT(CInfoBox::onPaint)
END_EVENT_TABLE()

CInfoBox::CInfoBox(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_width(size.GetWidth()),
m_height(size.GetHeight()),
m_bitmap(NULL)
{
	m_bitmap = new wxBitmap(m_width, m_height);

	// Flood the graph area with black to start with
	clearGraph();

	setTX(false);
	setRIT(false);
	setSplitShift(VFO_NONE);
}

CInfoBox::~CInfoBox()
{
	delete m_bitmap;
}

void CInfoBox::setVFO(int vfoNum)
{
	wxMemoryDC memoryDC;
	memoryDC.SelectObject(*m_bitmap);

	memoryDC.SetBrush(*wxBLACK_BRUSH);
	memoryDC.SetPen(*wxBLACK_PEN);
	memoryDC.DrawRectangle(0, 0, m_width, m_height / 4);

	wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
	font.SetPointSize(FONT_SIZE);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	memoryDC.SetFont(font);

	memoryDC.SetTextForeground(wxColour(0, 255, 255));

	switch (vfoNum) {
		case VFO_A:
			memoryDC.DrawText(wxT("VFO A"), 5, 2);
			break;
		case VFO_B:
			memoryDC.DrawText(wxT("VFO B"), 5, 2);
			break;
		case VFO_C:
			memoryDC.DrawText(wxT("VFO C"), 5, 2);
			break;
		case VFO_D:
			memoryDC.DrawText(wxT("VFO D"), 5, 2);
			break;
		default:
			wxASSERT(false);
			break;
	}

	memoryDC.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);
}

void CInfoBox::setRIT(bool onOff)
{
	wxMemoryDC memoryDC;
	memoryDC.SelectObject(*m_bitmap);

	memoryDC.SetBrush(*wxBLACK_BRUSH);
	memoryDC.SetPen(*wxBLACK_PEN);
	memoryDC.DrawRectangle(0, m_height / 4, m_width, m_height / 4);

	if (onOff) {
		wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
		font.SetPointSize(FONT_SIZE);
		font.SetWeight(wxFONTWEIGHT_BOLD);
		memoryDC.SetFont(font);

		memoryDC.SetTextForeground(wxColour(0, 255, 255));
		memoryDC.DrawText(wxT("RIT"), 5, m_height / 4 + 2);
	}

	memoryDC.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);
}

void CInfoBox::setSplitShift(int splitShift)
{
	wxMemoryDC memoryDC;
	memoryDC.SelectObject(*m_bitmap);

	memoryDC.SetBrush(*wxBLACK_BRUSH);
	memoryDC.SetPen(*wxBLACK_PEN);
	memoryDC.DrawRectangle(0, m_height / 2, m_width, m_height / 4);

	wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
	font.SetPointSize(FONT_SIZE);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	memoryDC.SetFont(font);
	memoryDC.SetTextForeground(wxColour(0, 255, 255));

	switch (splitShift) {
		case VFO_NONE:
			break;
		case VFO_SPLIT:
			memoryDC.DrawText(wxT("SPLIT"), 5, m_height / 2 + 2);
			break;
		case VFO_SHIFT1_PLUS:
			memoryDC.DrawText(wxT("SHIFT 1 +"), 5, m_height / 2 + 2);
			break;
		case VFO_SHIFT1_MINUS:
			memoryDC.DrawText(wxT("SHIFT 1 -"), 5, m_height / 2 + 2);
			break;
		case VFO_SHIFT2_PLUS:
			memoryDC.DrawText(wxT("SHIFT 2 +"), 5, m_height / 2 + 2);
			break;
		case VFO_SHIFT2_MINUS:
			memoryDC.DrawText(wxT("SHIFT 2 -"), 5, m_height / 2 + 2);
			break;
	}

	memoryDC.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);
}

void CInfoBox::setTX(bool onOff)
{
	wxMemoryDC memoryDC;
	memoryDC.SelectObject(*m_bitmap);

	memoryDC.SetBrush(*wxBLACK_BRUSH);
	memoryDC.SetPen(*wxBLACK_PEN);
	memoryDC.DrawRectangle(0, 3 * m_height / 4, m_width, m_height / 4);

	if (onOff) {
		wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
		font.SetPointSize(FONT_SIZE);
		font.SetWeight(wxFONTWEIGHT_BOLD);
		memoryDC.SetFont(font);

		memoryDC.SetTextForeground(wxColour(255, 0, 0));
		memoryDC.DrawText(wxT("TRANSMIT"), 5, 3 * m_height / 4 + 2);
	}

	memoryDC.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);
}

void CInfoBox::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	show(dc);
}

void CInfoBox::show(wxDC& dc)
{
	dc.DrawBitmap(*m_bitmap, 0, 0, false);
}

void CInfoBox::clearGraph()
{
	// Flood the graph area with black to start with
	wxMemoryDC dc;
	dc.SelectObject(*m_bitmap);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();
}
