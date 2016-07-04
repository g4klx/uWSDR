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

#include "SMeter.h"

#include <cmath>

enum {
	MENU_I_INPUT = 7863,
	MENU_Q_INPUT,
	MENU_SIGNAL,
	MENU_AVG_SIGNAL,
	MENU_MICROPHONE,
	MENU_POWER,
	MENU_ALC,
	MENU_COMPRESSED,
	MENU_RX_MENU,
	MENU_TX_MENU
};

BEGIN_EVENT_TABLE(CSMeter, wxPanel)
	EVT_RIGHT_DOWN(CSMeter::onMouse)
	EVT_MENU(MENU_I_INPUT,    CSMeter::onMenu)
	EVT_MENU(MENU_Q_INPUT,    CSMeter::onMenu)
	EVT_MENU(MENU_SIGNAL,     CSMeter::onMenu)
	EVT_MENU(MENU_AVG_SIGNAL, CSMeter::onMenu)
	EVT_MENU(MENU_MICROPHONE, CSMeter::onMenu)
	EVT_MENU(MENU_POWER,      CSMeter::onMenu)
	EVT_MENU(MENU_ALC,        CSMeter::onMenu)
	EVT_MENU(MENU_COMPRESSED, CSMeter::onMenu)
	EVT_PAINT(CSMeter::onPaint)
END_EVENT_TABLE()

CSMeter::CSMeter(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_width(size.GetWidth()),
m_height(size.GetHeight()),
m_rxBackground(NULL),
m_txBackground(NULL),
m_bitmap(NULL),
m_menu(NULL),
m_rxMenu(NULL),
m_txMenu(NULL),
m_rxMeter(METER_SIGNAL),
m_txMeter(METER_POWER),
m_lastLevel(999.9F),
m_txOn(false)
{
	m_bitmap       = new wxBitmap(m_width, m_height);
	m_rxBackground = new wxBitmap(m_width, m_height);
	m_txBackground = new wxBitmap(m_width, m_height);

	m_rxMenu = new wxMenu();
	m_rxMenu->AppendRadioItem(MENU_I_INPUT,    _("I Input"));
	m_rxMenu->AppendRadioItem(MENU_Q_INPUT,    _("Q Input"));
	m_rxMenu->AppendRadioItem(MENU_SIGNAL,     _("Strength"));
	m_rxMenu->AppendRadioItem(MENU_AVG_SIGNAL, _("Avg Strength"));

	m_txMenu = new wxMenu();
	m_txMenu->AppendRadioItem(MENU_MICROPHONE, _("Microphone"));
	m_txMenu->AppendRadioItem(MENU_COMPRESSED, _("Compressed"));
	m_txMenu->AppendRadioItem(MENU_ALC,        _("Post ALC"));
	m_txMenu->AppendRadioItem(MENU_POWER,      _("Power"));

	m_menu = new wxMenu();
	m_menu->Append(MENU_RX_MENU, _("Receive"),  m_rxMenu);
	m_menu->Append(MENU_TX_MENU, _("Transmit"), m_txMenu);

	createBackground();
	setLevel(0.0);
}

CSMeter::~CSMeter()
{
	delete m_bitmap;
	delete m_rxBackground;
	delete m_txBackground;
	delete m_menu;
}

void CSMeter::setTXMenu(bool set)
{
	m_menu->Enable(MENU_TX_MENU, set);
}

void CSMeter::setTX(bool on)
{
	m_txOn = on;
}

void CSMeter::setLevel(float level)
{
	if (level < 0.0F)
		level = 0.0F;

	if (level == m_lastLevel)
		return;

	wxMemoryDC dcBackground;
	if (m_txOn)
		dcBackground.SelectObject(*m_txBackground);
	else
		dcBackground.SelectObject(*m_rxBackground);

	wxMemoryDC dc;
	dc.SelectObject(*m_bitmap);

	dc.Blit(0, 0, m_width, m_height, &dcBackground, 0, 0);
	dcBackground.SelectObject(wxNullBitmap);

	// Draw the pointer
	dc.SetBrush(*wxWHITE_BRUSH);
	dc.SetPen(*wxWHITE_PEN);

	int centreX = SMETER_WIDTH / 2;
	int centreY = SMETER_WIDTH / 2 + 20;

	int endX;
	int endY;

	if (m_txOn) {
		if (level <= 0.625F) {
			double angle = (M_PI / 180.0) * double(45.0F - level * 72.0F);;
			endX = centreX - int((SMETER_WIDTH + 15) * ::sin(angle) / 2.0 + 0.5);
			endY = centreY - int((SMETER_WIDTH + 15) * ::cos(angle) / 2.0 + 0.5);
		} else {
			if (level > 1.25F)
				level = 1.25F;

			double angle = (M_PI / 180.0) * double(level - 0.625F) * 72.0;
			endX = centreX + int((SMETER_WIDTH + 15) * ::sin(angle) / 2.0 + 0.5);
			endY = centreY - int((SMETER_WIDTH + 15) * ::cos(angle) / 2.0 + 0.5);
		}
	} else {
		if (level <= 54.0F) {	// S0 to S9
			double angle = (M_PI / 180.0) * double(45.0F - level * 0.833333F);
			endX = centreX - int((SMETER_WIDTH + 15) * ::sin(angle) / 2.0 + 0.5);
			endY = centreY - int((SMETER_WIDTH + 15) * ::cos(angle) / 2.0 + 0.5);
		} else {			// dB over S9
			if (level > 94.0F)	// 40dB over S9
				level = 94.0F;

			double angle = (M_PI / 180.0) * double(level - 54.0F) * 1.125;
			endX = centreX + int((SMETER_WIDTH + 15) * ::sin(angle) / 2.0 + 0.5);
			endY = centreY - int((SMETER_WIDTH + 15) * ::cos(angle) / 2.0 + 0.5);
		}
	}

	dc.DrawLine(centreX, centreY, endX, endY);

	dc.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);

	m_lastLevel = level;
}

void CSMeter::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	show(dc);
}

void CSMeter::show(wxDC& dc)
{
	dc.DrawBitmap(*m_bitmap, 0, 0, false);
}

void CSMeter::createBackground()
{
	// Flood the graph area with black to start with
	wxMemoryDC rxDC, txDC;

	rxDC.SelectObject(*m_rxBackground);
	txDC.SelectObject(*m_txBackground);

	rxDC.SetBackground(*wxBLACK_BRUSH);
	txDC.SetBackground(*wxBLACK_BRUSH);
	rxDC.Clear();
	txDC.Clear();

	rxDC.SetBrush(*wxGREEN_BRUSH);
	rxDC.SetPen(*wxBLACK_PEN);
	rxDC.DrawEllipticArc(0, 20, SMETER_WIDTH, SMETER_WIDTH, 90.0, 135.0);

	rxDC.SetBrush(*wxRED_BRUSH);
	rxDC.SetPen(*wxBLACK_PEN);
	rxDC.DrawEllipticArc(0, 20, SMETER_WIDTH, SMETER_WIDTH, 45.0, 90.0);

	txDC.SetBrush(*wxGREEN_BRUSH);
	txDC.SetPen(*wxBLACK_PEN);
	txDC.DrawEllipticArc(0, 20, SMETER_WIDTH, SMETER_WIDTH, 45.0, 135.0);

	rxDC.SetPen(*wxWHITE_PEN);
	txDC.SetPen(*wxWHITE_PEN);

	wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
	font.SetPointSize(11);
	font.SetWeight(wxFONTWEIGHT_BOLD);

	rxDC.SetFont(font);
	txDC.SetFont(font);

	rxDC.SetTextForeground(*wxWHITE);
	txDC.SetTextForeground(*wxWHITE);

	int centreX = SMETER_WIDTH / 2;
	int centreY = SMETER_WIDTH / 2 + 20;

	// S0
	int endX = centreX - int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	int endY = centreY - int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	rxDC.DrawLine(centreX, centreY, endX, endY);
	rxDC.DrawText(wxT("0"), endX - 10, endY - 10);
	// S3
	endX = centreX - int((SMETER_WIDTH + 15) * 0.25 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.43301 + 0.5);
	rxDC.DrawLine(centreX, centreY, endX, endY);
	rxDC.DrawText(wxT("3"), endX - 6, endY - 12);
	// S6
	endX = centreX - int((SMETER_WIDTH + 15) * 0.12941 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.48296 + 0.5);
	rxDC.DrawLine(centreX, centreY, endX, endY);
	rxDC.DrawText(wxT("6"), endX - 5, endY - 14);
	// S9
	endX = centreX;
	endY = 15;
	rxDC.DrawLine(centreX, centreY, endX, endY);
	rxDC.DrawText(wxT("9"), endX - 5, endY - 15);
	// S9+20
	endX = centreX + int((SMETER_WIDTH + 15) * 0.19134 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.46194 + 0.5);
	rxDC.DrawLine(centreX, centreY, endX, endY);
	rxDC.DrawText(wxT("+20"), endX - 5, endY - 12);
	// S9+40
	endX = centreX + int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	rxDC.DrawLine(centreX, centreY, endX, endY);
	rxDC.DrawText(wxT("+40"), endX - 5, endY - 10);

	// 0
	endX = centreX - int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	txDC.DrawLine(centreX, centreY, endX, endY);
	txDC.DrawText(wxT("0"), endX - 10, endY - 10);
	// 2.5
	endX = centreX - int((SMETER_WIDTH + 15) * 0.19134 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.46194 + 0.5);
	txDC.DrawLine(centreX, centreY, endX, endY);
	// 5
	endX = centreX + int((SMETER_WIDTH + 15) * 0.0 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.5 + 0.5);
	txDC.DrawLine(centreX, centreY, endX, endY);
	// 7.5
	endX = centreX + int((SMETER_WIDTH + 15) * 0.19134 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.46194 + 0.5);
	txDC.DrawLine(centreX, centreY, endX, endY);
	// 10
	endX = centreX + int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	endY = centreY - int((SMETER_WIDTH + 15) * 0.35355 + 0.5);
	txDC.DrawLine(centreX, centreY, endX, endY);
	txDC.DrawText(wxT("10"), endX, endY - 10);

	rxDC.SetBrush(*wxBLACK_BRUSH);
	rxDC.SetPen(*wxBLACK_PEN);
	rxDC.DrawEllipse(0, 25, SMETER_WIDTH, SMETER_WIDTH);

	txDC.SetBrush(*wxBLACK_BRUSH);
	txDC.SetPen(*wxBLACK_PEN);
	txDC.DrawEllipse(0, 25, SMETER_WIDTH, SMETER_WIDTH);

	rxDC.SelectObject(wxNullBitmap);
	txDC.SelectObject(wxNullBitmap);
}

void CSMeter::onMouse(wxMouseEvent& event)
{
	switch (m_rxMeter) {
		case METER_I_INPUT:
			m_rxMenu->Check(MENU_I_INPUT, true);
			break;
		case METER_Q_INPUT:
			m_rxMenu->Check(MENU_Q_INPUT, true);
			break;
		case METER_SIGNAL:
			m_rxMenu->Check(MENU_SIGNAL, true);
			break;
		case METER_AVG_SIGNAL:
			m_rxMenu->Check(MENU_AVG_SIGNAL, true);
			break;
		default:
			wxLogError(wxT("Unknown RX meter type = %d"), m_rxMeter);
			break;
	}

	switch (m_txMeter) {
		case METER_MICROPHONE:
			m_txMenu->Check(MENU_MICROPHONE, true);
			break;
		case METER_POWER:
			m_txMenu->Check(MENU_POWER, true);
			break;
		case METER_ALC:
			m_txMenu->Check(MENU_ALC, true);
			break;
		case METER_COMPRESSED:
			m_txMenu->Check(MENU_COMPRESSED, true);
			break;
		default:
			wxLogError(wxT("Unknown TX meter type = %d"), m_txMeter);
			break;
	}

	int x = event.GetX();
	int y = event.GetY();

	PopupMenu(m_menu, x, y);
}

void CSMeter::onMenu(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case MENU_I_INPUT:
			setRXMeter(METER_I_INPUT);
			break;
		case MENU_Q_INPUT:
			setRXMeter(METER_Q_INPUT);
			break;
		case MENU_SIGNAL:
			setRXMeter(METER_SIGNAL);
			break;
		case MENU_AVG_SIGNAL:
			setRXMeter(METER_AVG_SIGNAL);
			break;
		case MENU_MICROPHONE:
			setTXMeter(METER_MICROPHONE);
			break;
		case MENU_POWER:
			setTXMeter(METER_POWER);
			break;
		case MENU_ALC:
			setTXMeter(METER_ALC);
			break;
		case MENU_COMPRESSED:
			setTXMeter(METER_COMPRESSED);
			break;
	}
}

void CSMeter::setRXMeter(METERPOS meter)
{
	m_rxMeter = meter;
}

void CSMeter::setTXMeter(METERPOS meter)
{
	m_txMeter = meter;
}

METERPOS CSMeter::getRXMeter() const
{
	return m_rxMeter;
}

METERPOS CSMeter::getTXMeter() const
{
	return m_txMeter;
}
