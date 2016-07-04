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

#include "SpectrumDisplay.h"

#include <wx/image.h>

enum {
	MENU_PANADAPTER1 = 16531,
	MENU_PANADAPTER2,
	MENU_WATERFALL,
	MENU_PHASE,
	MENU_AUDIO,
	MENU_200MS,
	MENU_400MS,
	MENU_600MS,
	MENU_800MS,
	MENU_1000MS,
	MENU_10DB,
	MENU_20DB,
	MENU_30DB,
	MENU_40DB,
	MENU_50DB,
	MENU_60DB
};

BEGIN_EVENT_TABLE(CSpectrumDisplay, wxPanel)
	EVT_PAINT(CSpectrumDisplay::onPaint)
	EVT_LEFT_DOWN(CSpectrumDisplay::onLeftMouse)
	EVT_RIGHT_DOWN(CSpectrumDisplay::onRightMouse)
	EVT_MENU(MENU_PANADAPTER1, CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_PANADAPTER2, CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_WATERFALL,   CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_PHASE,       CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_AUDIO,       CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_200MS,       CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_400MS,       CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_600MS,       CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_800MS,       CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_1000MS,      CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_10DB,        CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_20DB,        CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_30DB,        CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_40DB,        CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_50DB,        CSpectrumDisplay::onMenu)
	EVT_MENU(MENU_60DB,        CSpectrumDisplay::onMenu)
END_EVENT_TABLE()


CSpectrumDisplay::CSpectrumDisplay(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_width(size.GetWidth()),
m_height(size.GetHeight()),
m_dbScale(40.0F),
m_background(NULL),
m_bitmap(NULL),
m_sampleRate(0.0F),
m_bandwidth(0.0F),
m_highFilter(0.0F),
m_lowFilter(0.0F),
m_menu(NULL),
m_speedMenu(NULL),
m_typeMenu(NULL),
m_dbMenu(NULL),
m_type(SPECTRUM_PANADAPTER1),
m_speed(SPECTRUM_200MS),
m_db(SPECTRUM_50DB),
m_factor(1),
m_ticks(0),
m_pick(0.0F),
m_offset(0.0F)
{
	m_bitmap     = new wxBitmap(m_width, m_height);
	m_background = new wxBitmap(m_width, m_height);

	m_speedMenu = new wxMenu();
	m_speedMenu->AppendRadioItem(MENU_200MS,  _("200 ms"));
	m_speedMenu->AppendRadioItem(MENU_400MS,  _("400 ms"));
	m_speedMenu->AppendRadioItem(MENU_600MS,  _("600 ms"));
	m_speedMenu->AppendRadioItem(MENU_800MS,  _("800 ms"));
	m_speedMenu->AppendRadioItem(MENU_1000MS, _("1 s"));

	m_dbMenu = new wxMenu();
	m_dbMenu->AppendRadioItem(MENU_10DB, _("10 dB"));
	m_dbMenu->AppendRadioItem(MENU_20DB, _("20 dB"));
	m_dbMenu->AppendRadioItem(MENU_30DB, _("30 dB"));
	m_dbMenu->AppendRadioItem(MENU_40DB, _("40 dB"));
	m_dbMenu->AppendRadioItem(MENU_50DB, _("50 dB"));
	m_dbMenu->AppendRadioItem(MENU_60DB, _("60 dB"));

	m_typeMenu = new wxMenu();
	m_typeMenu->AppendRadioItem(MENU_PANADAPTER1, _("Panadapter 1"));
	m_typeMenu->AppendRadioItem(MENU_PANADAPTER2, _("Panadapter 2"));
	m_typeMenu->AppendRadioItem(MENU_WATERFALL,   _("Waterfall"));
	m_typeMenu->AppendRadioItem(MENU_PHASE,       _("IQ Phase"));
	m_typeMenu->AppendRadioItem(MENU_AUDIO,       _("Audio"));

	m_menu = new wxMenu();
	m_menu->Append(-1, _("Type"),     m_typeMenu);
	m_menu->Append(-1, _("Refresh"),  m_speedMenu);
	m_menu->Append(-1, _("dB Range"), m_dbMenu);
}

CSpectrumDisplay::~CSpectrumDisplay()
{
	delete m_background;
	delete m_bitmap;
	delete m_menu;
}

void CSpectrumDisplay::setSampleRate(float sampleRate)
{
	m_sampleRate = sampleRate;
}

void CSpectrumDisplay::setBandwidth(float bandwidth)
{
	m_bandwidth = bandwidth;

	switch (m_type) {
		case SPECTRUM_PANADAPTER1:
		case SPECTRUM_PANADAPTER2:
			createPanadapter();
			break;
		case SPECTRUM_WATERFALL:
			createWaterfall();
			break;
		case SPECTRUM_PHASE:
			createPhase();
			break;
		case SPECTRUM_AUDIO:
			createScope();
			break;
		default:
			wxLogError(wxT("Unknown spectrum type = %d"), m_type);
			break;
	}

	wxClientDC clientDC(this);
	show(clientDC);
}

void CSpectrumDisplay::showSpectrum(const float* spectrum, float bottom, float offset)
{
	wxASSERT(spectrum != NULL);

	m_offset = offset;

	if ((m_ticks % m_factor) == 0) {
		switch (m_type) {
			case SPECTRUM_PANADAPTER1:
				drawPanadapter1(spectrum, bottom);
				break;
			case SPECTRUM_PANADAPTER2:
				drawPanadapter2(spectrum, bottom);
				break;
			case SPECTRUM_WATERFALL:
				drawWaterfall(spectrum, bottom);
				break;
			case SPECTRUM_PHASE:
				drawPhase(spectrum);
				break;
			case SPECTRUM_AUDIO:
				drawScope(spectrum);
				break;
			default:
				break;
		}

		wxClientDC clientDC(this);
		show(clientDC);
	}

	m_ticks++;
}

void CSpectrumDisplay::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	show(dc);
}

void CSpectrumDisplay::show(wxDC& dc)
{
	dc.DrawBitmap(*m_bitmap, 0, 0, false);
}

void CSpectrumDisplay::createPanadapter()
{
	// Flood the graph area with black to start with
	wxMemoryDC dc;
	dc.SelectObject(*m_background);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();

	if (m_lowFilter != 0.0F || m_highFilter != 0.0F) {
		dc.SetPen(*wxMEDIUM_GREY_PEN);
		dc.SetBrush(*wxMEDIUM_GREY_BRUSH);

		wxCoord rxLow  = int(float(m_width - 5) / m_bandwidth * m_lowFilter) + m_width / 2;
		wxCoord rxHigh = int(float(m_width - 5) / m_bandwidth * m_highFilter) + m_width / 2;

		dc.DrawRectangle(rxLow, 2, rxHigh - rxLow, m_height - 16);
	}

	int left    = 2;
	int right   = m_width - 3;
	int top     = 2;
	int bottom  = m_height - 15;
	int middleX = left + (right - left) / 2;
	double incrX = double(right - left) / 10.0;

	dc.SetPen(*wxCYAN_PEN);

	dc.DrawLine(left, top, left, bottom);
	dc.DrawLine(right, top, right, bottom);
	dc.DrawLine(middleX, top, middleX, bottom);
	dc.DrawLine(left, bottom, right, bottom);
	dc.DrawLine(left, top, right, top);

	wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
	font.SetPointSize(11);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(font);

	dc.SetTextForeground(*wxCYAN);

	// Draw the frequency lines
	for (unsigned int i = 1U; i < 10U; i++) {
		int x = left + int(i * incrX + 0.5);
		dc.DrawLine(x, top /* lowY */, x, bottom);
	}

	float dbScale = float(m_height - 15) / m_dbScale;

	unsigned int dbIncr = 10U;
	if (m_dbScale > 50.0F)
		dbIncr = 20U;
	else if (m_dbScale < 30.0F)
		dbIncr = 5U;

	// Draw the dB lines, every 10 dB
	for (unsigned int dB = 0U; true; dB += dbIncr) {
		int y = bottom - int(float(dB) * dbScale + 0.5);
		if (y < top)
			break;

		dc.DrawLine(left, y, right, y);

		wxString text;
		text.Printf(wxT("%u"), dB);

		wxCoord width, height;
		dc.GetTextExtent(text, &width, &height);

		dc.DrawText(text, left + 2, y - height);
		dc.DrawText(text, right - width - 2, y - height);
	}

	wxString text;
	text.Printf(wxT("-%.1f kHz"), m_bandwidth / 2000.0F);
	dc.DrawText(text, left, bottom);

	text = wxT("0");
	wxCoord height, width;
	dc.GetTextExtent(text, &width, &height);
	dc.DrawText(text, middleX - width / 2, bottom);

	text.Printf(wxT("+%.1f kHz"), m_bandwidth / 2000.0F);
	dc.GetTextExtent(text, &width, &height);
	dc.DrawText(text, right - width, bottom);

	wxMemoryDC dcScreen;
	dcScreen.SelectObject(*m_bitmap);

	dcScreen.Blit(0, 0, m_width, m_height, &dc, 0, 0);

	dcScreen.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::createWaterfall()
{
	// Flood the graph area with black to start with
	wxMemoryDC dc;
	dc.SelectObject(*m_bitmap);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();

	int left    = 2;
	int right   = m_width - 3;
	int bottom  = m_height - 15;
	int middleX = left + (right - left) / 2;

	dc.SetPen(*wxCYAN_PEN);

	dc.DrawLine(left, bottom, right, bottom);

	wxFont font = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
	font.SetPointSize(11);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(font);

	dc.SetTextForeground(*wxCYAN);

	wxString text;
	text.Printf(wxT("-%.1f kHz"), m_bandwidth / 2000.0F);
	dc.DrawText(text, left, bottom);

	text = wxT("0");
	wxCoord height, width;
	dc.GetTextExtent(text, &width, &height);
	dc.DrawText(text, middleX - width / 2, bottom);

	text.Printf(wxT("+%.1f kHz"), m_bandwidth / 2000.0F);
	dc.GetTextExtent(text, &width, &height);
	dc.DrawText(text, right - width, bottom);

	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::createPhase()
{
	// Flood the graph area with black to start with
	wxMemoryDC dc;
	dc.SelectObject(*m_background);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();

	int left   = 0;
	int right  = m_width;
	int top    = 2;
	int bottom = m_height - 3;
	int mid    = (bottom - top) / 2;

	dc.SetPen(*wxCYAN_PEN);
	dc.DrawLine(left, mid, right, mid);

	wxMemoryDC dcScreen;
	dcScreen.SelectObject(*m_bitmap);

	dcScreen.Blit(0, 0, m_width, m_height, &dc, 0, 0);

	dcScreen.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::createScope()
{
	// Flood the graph area with black to start with
	wxMemoryDC dc;
	dc.SelectObject(*m_background);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();

	int left   = 0;
	int right  = m_width;
	int top    = 2;
	int bottom = m_height - 3;
	int mid    = (bottom - top) / 2;

	dc.SetPen(*wxCYAN_PEN);
	dc.DrawLine(left, mid, right, mid);

	wxMemoryDC dcScreen;
	dcScreen.SelectObject(*m_bitmap);

	dcScreen.Blit(0, 0, m_width, m_height, &dc, 0, 0);

	dcScreen.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::drawPanadapter1(const float* spectrum, float bottom)
{
	wxMemoryDC dc, dcBack;

	dc.SelectObject(*m_bitmap);
	dcBack.SelectObject(*m_background);

	dc.Blit(0, 0, m_width, m_height, &dcBack, 0, 0);

	dc.SetPen(*wxGREEN_PEN);

	int firstBin = int(float(SPECTRUM_SIZE) / 2.0F - m_bandwidth / 2.0F * float(SPECTRUM_SIZE) / m_sampleRate + 0.5);
	int lastBin  = int(float(SPECTRUM_SIZE) / 2.0F + m_bandwidth / 2.0F * float(SPECTRUM_SIZE) / m_sampleRate + 0.5);

	float binsPerPixel = float(lastBin - firstBin) / float(m_width - 5);

	float dbScale = float(m_height - 15) / m_dbScale;

	int binOffset = int(float(lastBin - firstBin) * (m_offset / m_bandwidth) + 0.5F);
	firstBin += binOffset;
	lastBin  += binOffset;

	int lastX = 0, lastY = 0;
	for (int x = 2; x < (m_width - 3); x++) {
		int bin = firstBin + int(float(x - 2) * binsPerPixel + 0.5F);

		float value = -200.0F;
		for (int i = 0; i < int(binsPerPixel + 0.5F); i++) {
			float val = spectrum[bin++];

			if (val > value)
				value = val;
		}

		value -= bottom;

		int y = int(value * 0.5F * dbScale + 0.5F);
		if (y < 2)
			y = 2;
		if (y > (m_height - 18))
			y = m_height - 18;

		y = m_height - 15 - y;

		if (x > 2)
			dc.DrawLine(lastX, lastY, x, y);

		lastX = x;
		lastY = y;
	}

	dcBack.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::drawPanadapter2(const float* spectrum, float bottom)
{
	wxMemoryDC dc, dcBack;

	dc.SelectObject(*m_bitmap);
	dcBack.SelectObject(*m_background);

	dc.Blit(0, 0, m_width, m_height, &dcBack, 0, 0);

	dc.SetPen(*wxGREEN_PEN);

	int firstBin = int(float(SPECTRUM_SIZE) / 2.0F - m_bandwidth / 2.0F * float(SPECTRUM_SIZE) / m_sampleRate + 0.5);
	int lastBin  = int(float(SPECTRUM_SIZE) / 2.0F + m_bandwidth / 2.0F * float(SPECTRUM_SIZE) / m_sampleRate + 0.5);

	float binsPerPixel = float(lastBin - firstBin) / float(m_width - 5);

	float dbScale = float(m_height - 15) / m_dbScale;

	int binOffset = int(float(lastBin - firstBin) * (m_offset / m_bandwidth) + 0.5F);
	firstBin += binOffset;
	lastBin  += binOffset;

	for (int x = 2; x < (m_width - 3); x++) {
		int bin = firstBin + int(float(x - 2) * binsPerPixel + 0.5F);

		float value = -200.0F;
		for (int i = 0; i < int(binsPerPixel + 0.5F); i++) {
			float val = spectrum[bin++];

			if (val > value)
				value = val;
		}

		value -= bottom;

		int y = int(value * 0.5F * dbScale + 0.5F);
		if (y < 2)
			y = 2;
		if (y > (m_height - 18))
			y = m_height - 18;

		y = m_height - 15 - y;

		dc.DrawLine(x, m_height - 15, x, y);
	}

	dcBack.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

/*
 * Could we gain speed by storing the wxImage instead of the wxBitmap?
 */
void CSpectrumDisplay::drawWaterfall(const float* spectrum, float bottom)
{
	int firstBin = int(float(SPECTRUM_SIZE) / 2.0F - m_bandwidth / 2.0F * float(SPECTRUM_SIZE) / m_sampleRate + 0.5);
	int lastBin  = int(float(SPECTRUM_SIZE) / 2.0F + m_bandwidth / 2.0F * float(SPECTRUM_SIZE) / m_sampleRate + 0.5);

	float binsPerPixel = float(lastBin - firstBin) / float(m_width - 5);

	int binOffset = int(float(lastBin - firstBin) * (m_offset / m_bandwidth) + 0.5F);
	firstBin += binOffset;
	lastBin  += binOffset;

	// Do all of the work on a wxImage
	wxImage image = m_bitmap->ConvertToImage();

	unsigned char* data = image.GetData();

	// Scroll the image up by one pixel
	::memcpy(data, data + m_width * 3, (m_height - 16) * m_width * 3);

	unsigned char* imgOffset = data + m_width * (m_height - 16) * 3;

	for (int x = 2; x < (m_width - 3); x++) {
		int bin = firstBin + int(float(x - 2) * binsPerPixel + 0.5);

		float value = -200.0F;
		for (int i = 0; i < int(binsPerPixel + 0.5); i++) {
			float val = spectrum[bin++];

			if (val > value)
				value = val;
		}

		value -= bottom;

		float percent = 0.5F * value / m_dbScale;
		if (percent < 0.0F)
			percent = 0.0F;
		if (percent > 1.0F)
			percent = 1.0F;

		unsigned char r;
		unsigned char g;
		unsigned char b;

		if (percent <= 0.33333333F) {			// use a gradient between low and mid colors
			percent *= 3.0F;

			b = (unsigned char)(percent * 255.0F + 0.5F);
			g = 0;
			r = 0;
			// r = (unsigned char)(percent * 255.0F + 0.5F);
			// g = 0;
			// b = 0;
		} else if (percent <= 0.66666666F) {		// use a gradient between mid and high colors
			percent = (percent - 0.33333333F) * 3.0F;

			b = 255;
			g = (unsigned char)(percent * 255.0F + 0.5F);
			r = 0;
			// r = 255;
			// g = (unsigned char)(percent * 255.0F + 0.5F);
			// b = 0;
		} else {
			percent = (percent - 0.66666666F) * 3.0F;

			b = 255;
			g = 255;
			r = (unsigned char)(percent * 255.0F + 0.5F);
			// r = 255;
			// g = 255;
			// b = (unsigned char)(percent * 255.0F + 0.5F);
		}

		*imgOffset++ = r;
		*imgOffset++ = g;
		*imgOffset++ = b;
	}

	delete m_bitmap;
	m_bitmap = new wxBitmap(image);
}

void CSpectrumDisplay::drawPhase(const float* spectrum)
{
	wxMemoryDC dc, dcBack;

	dc.SelectObject(*m_bitmap);
	dcBack.SelectObject(*m_background);

	dc.Blit(0, 0, m_width, m_height, &dcBack, 0, 0);

	int left   = 0;
	int right  = m_width;
	int top    = 2;
	int bottom = m_height - 3;
	int mid    = (bottom - top) / 2;

	float max = 0.0F;
	for (int i = 0; i < (right - left); i++) {
		float data1 = ::fabs(spectrum[i * 2 + 0]);		// I
		float data2 = ::fabs(spectrum[i * 2 + 1]);		// Q

		if (data1 > max)
			max = data1;
		if (data2 > max)
			max = data2;
	}

	float scale = float(mid) / max;

	int lastX1 = 0, lastY1 = 0;
	int lastX2 = 0, lastY2 = 0;
	for (int i = 0; i < (right - left); i++) {
		float data1 = spectrum[i * 2 + 0];		// I
		float data2 = spectrum[i * 2 + 1];		// Q

		if (data1 >= 1.0F)
			data1 = 1.0F;
		else if (data1 <= -1.0F)
			data1 = -1.0F;

		if (data2 >= 1.0F)
			data2 = 1.0F;
		else if (data2 <= -1.0F)
			data2 = -1.0F;

		int x = i + left;

		int y1 = mid + int(data1 * scale);
		int y2 = mid + int(data2 * scale);

		if (i > 0) {
			dc.SetPen(*wxGREEN_PEN);
			dc.DrawLine(lastX1, lastY1, x, y1);

			dc.SetPen(*wxCYAN_PEN);
			dc.DrawLine(lastX2, lastY2, x, y2);
		}

		lastX1 = x;
		lastY1 = y1;

		lastX2 = x;
		lastY2 = y2;
	}

	dcBack.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::drawScope(const float* spectrum)
{
	wxMemoryDC dc, dcBack;

	dc.SelectObject(*m_bitmap);
	dcBack.SelectObject(*m_background);

	dc.Blit(0, 0, m_width, m_height, &dcBack, 0, 0);

	dc.SetPen(*wxGREEN_PEN);

	int left   = 0;
	int right  = m_width;
	int top    = 2;
	int bottom = m_height -  3;
	int mid    = (bottom - top) / 2;

	int lastX = 0, lastY = 0;
	for (int i = 0; i < (right - left); i++) {
		float data = spectrum[i];

		if (data >= 1.0F)
			data = 1.0F;
		else if (data <= -1.0F)
			data = -1.0F;

		int x = i + left;

		int y = mid + int(data * float(mid));

		if (i > 0)
			dc.DrawLine(lastX, lastY, x, y);

		lastX = x;
		lastY = y;
	}

	dcBack.SelectObject(wxNullBitmap);
	dc.SelectObject(wxNullBitmap);
}

void CSpectrumDisplay::onLeftMouse(wxMouseEvent& event)
{
	if (m_type == SPECTRUM_PHASE || m_type == SPECTRUM_AUDIO)
		return;

	float x     = float(event.GetX() - 2);
	float width = float(m_width - 5);

	m_pick = m_bandwidth / 2.0F - m_bandwidth * x / width;

	if (m_pick < -m_bandwidth / 2.0F) {
		m_pick = 0.0F;
		return;
	} else if (m_pick > m_bandwidth / 2.0F) {
		m_pick = 0.0F;
		return;
	}

	m_pick += (m_highFilter + m_lowFilter) / 2.0F;

	if (m_pick < -m_bandwidth / 2.0F)
		m_pick = -m_bandwidth / 2.0F;
	else if (m_pick > m_bandwidth / 2.0F)
		m_pick = m_bandwidth / 2.0F;
}

void CSpectrumDisplay::onRightMouse(wxMouseEvent& event)
{
	switch (m_type) {
		case SPECTRUM_PANADAPTER1:
			m_typeMenu->Check(MENU_PANADAPTER1, true);
			break;
		case SPECTRUM_PANADAPTER2:
			m_typeMenu->Check(MENU_PANADAPTER2, true);
			break;
		case SPECTRUM_WATERFALL:
			m_typeMenu->Check(MENU_WATERFALL, true);
			break;
		case SPECTRUM_PHASE:
			m_typeMenu->Check(MENU_PHASE, true);
			break;
		case SPECTRUM_AUDIO:
			m_typeMenu->Check(MENU_AUDIO, true);
			break;
		default:
			wxLogError(wxT("Unknown spectrum type = %d"), m_type);
			break;
	}

	switch (m_speed) {
		case SPECTRUM_200MS:
			m_speedMenu->Check(MENU_200MS, true);
			break;
		case SPECTRUM_400MS:
			m_speedMenu->Check(MENU_400MS, true);
			break;
		case SPECTRUM_600MS:
			m_speedMenu->Check(MENU_600MS, true);
			break;
		case SPECTRUM_800MS:
			m_speedMenu->Check(MENU_800MS, true);
			break;
		case SPECTRUM_1000MS:
			m_speedMenu->Check(MENU_1000MS, true);
			break;
		default:
			wxLogError(wxT("Unknown spectrum speed = %d"), m_speed);
			break;
	}

	switch (m_db) {
		case SPECTRUM_10DB:
			m_dbMenu->Check(MENU_10DB, true);
			break;
		case SPECTRUM_20DB:
			m_dbMenu->Check(MENU_20DB, true);
			break;
		case SPECTRUM_30DB:
			m_dbMenu->Check(MENU_30DB, true);
			break;
		case SPECTRUM_40DB:
			m_dbMenu->Check(MENU_40DB, true);
			break;
		case SPECTRUM_50DB:
			m_dbMenu->Check(MENU_50DB, true);
			break;
		case SPECTRUM_60DB:
			m_dbMenu->Check(MENU_60DB, true);
			break;
		default:
			wxLogError(wxT("Unknown dB range = %d"), m_db);
			break;
	}

	int x = event.GetX();
	int y = event.GetY();

	PopupMenu(m_menu, x, y);
}

void CSpectrumDisplay::onMenu(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case MENU_PANADAPTER1:
			setType(SPECTRUM_PANADAPTER1);
			break;
		case MENU_PANADAPTER2:
			setType(SPECTRUM_PANADAPTER2);
			break;
		case MENU_WATERFALL:
			setType(SPECTRUM_WATERFALL);
			break;
		case MENU_PHASE:
			setType(SPECTRUM_PHASE);
			break;
		case MENU_AUDIO:
			setType(SPECTRUM_AUDIO);
			break;
		case MENU_200MS:
			setSpeed(SPECTRUM_200MS);
			break;
		case MENU_400MS:
			setSpeed(SPECTRUM_400MS);
			break;
		case MENU_600MS:
			setSpeed(SPECTRUM_600MS);
			break;
		case MENU_800MS:
			setSpeed(SPECTRUM_800MS);
			break;
		case MENU_1000MS:
			setSpeed(SPECTRUM_1000MS);
			break;
		case MENU_10DB:
			setDB(SPECTRUM_10DB);
			break;
		case MENU_20DB:
			setDB(SPECTRUM_20DB);
			break;
		case MENU_30DB:
			setDB(SPECTRUM_30DB);
			break;
		case MENU_40DB:
			setDB(SPECTRUM_40DB);
			break;
		case MENU_50DB:
			setDB(SPECTRUM_50DB);
			break;
		case MENU_60DB:
			setDB(SPECTRUM_60DB);
			break;
		default:
			wxLogError(wxT("Unknown spectrum menu item = %d"), event.GetId());
			break;
	}
}

void CSpectrumDisplay::setFilter(FILTERWIDTH filter, UWSDRMODE mode)
{
	m_highFilter = 0.0F;
	m_lowFilter  = 0.0F;

	double width = 0.0;
	switch (filter) {
		case FILTER_20000:
			width = 20000.0;
			break;
		case FILTER_15000:
			width = 15000.0;
			break;
		case FILTER_10000:
			width = 10000.0;
			break;
		case FILTER_6000:
			width = 6000.0;
			break;
		case FILTER_4000:
			width = 4000.0;
			break;
		case FILTER_2600:
			width = 2600.0;
			break;
		case FILTER_2100:
			width = 2100.0;
			break;
		case FILTER_1000:
			width = 1000.0;
			break;
		case FILTER_500:
			width = 500.0;
			break;
		case FILTER_250:
			width = 250.0;
			break;
		case FILTER_100:
			width = 100.0;
			break;
		case FILTER_50:
			width = 50.0;
			break;
		case FILTER_25:
			width = 25.0;
			break;
		case FILTER_AUTO:
			return;
	}

	switch (mode) {
		case MODE_FMW:
		case MODE_FMN:
		case MODE_AM:
			m_highFilter =  width / 2.0;
			m_lowFilter  = -width / 2.0;
			break;

		case MODE_USB:
		case MODE_LSB:
		case MODE_DIGL:
		case MODE_DIGU:
			switch (filter) {
				case FILTER_20000:
				case FILTER_15000:
				case FILTER_10000:
				case FILTER_6000:
				case FILTER_4000:
					m_highFilter = width + 100.0;
					m_lowFilter  = 100.0;
					break;
				case FILTER_2600:
				case FILTER_2100:
				case FILTER_1000:
					m_highFilter = width + 200.0;
					m_lowFilter  = 200.0;
					break;
				case FILTER_500:
					m_highFilter = 850.0;
					m_lowFilter  = 350.0;
					break;
				case FILTER_250:
					m_highFilter = 725.0;
					m_lowFilter  = 475.0;
					break;
				case FILTER_100:
					m_highFilter = 650.0;
					m_lowFilter  = 550.0;
					break;
				case FILTER_50:
					m_highFilter = 625.0;
					m_lowFilter  = 575.0;
					break;
				case FILTER_25:
					m_highFilter = 613.0;
					m_lowFilter  = 587.0;
					break;
				case FILTER_AUTO:
					return;
			}
			break;

		case MODE_CWUW:
		case MODE_CWUN:
		case MODE_CWLW:
		case MODE_CWLN:
			switch (filter) {
				case FILTER_20000:
				case FILTER_15000:
				case FILTER_10000:
				case FILTER_6000:
				case FILTER_4000:
				case FILTER_2600:
				case FILTER_2100:
					m_highFilter = width + 100.0;
					m_lowFilter  = 100.0;
					break;
				case FILTER_1000:
					m_highFilter = CW_OFFSET + 500.0;
					m_lowFilter  = CW_OFFSET - 500.0;
					break;
				case FILTER_500:
					m_highFilter = CW_OFFSET + 250.0;
					m_lowFilter  = CW_OFFSET - 250.0;
					break;
				case FILTER_250:
					m_highFilter = CW_OFFSET + 125.0;
					m_lowFilter  = CW_OFFSET - 125.0;
					break;
				case FILTER_100:
					m_highFilter = CW_OFFSET + 50.0;
					m_lowFilter  = CW_OFFSET - 50.0;
					break;
				case FILTER_50:
					m_highFilter = CW_OFFSET + 25.0;
					m_lowFilter  = CW_OFFSET - 25.0;
					break;
				case FILTER_25:
					m_highFilter = CW_OFFSET + 13.0;
					m_lowFilter  = CW_OFFSET - 12.0;
					break;
				case FILTER_AUTO:
					return;
			}
			break;
	}

	// Swap the filter values over
	if (mode == MODE_LSB || mode == MODE_CWLN || mode == MODE_CWLW || mode == MODE_DIGL) {
		double swap;

		swap   = m_lowFilter;
		m_lowFilter  = -m_highFilter;
		m_highFilter = -swap;
	}

	createPanadapter();
}

void CSpectrumDisplay::setType(SPECTRUMTYPE type)
{
	if (type == m_type)
		return;

	switch (type) {
		case SPECTRUM_PANADAPTER1:
		case SPECTRUM_PANADAPTER2:
			createPanadapter();
			break;
		case SPECTRUM_WATERFALL:
			createWaterfall();
			break;
		case SPECTRUM_PHASE:
			createPhase();
			break;
		case SPECTRUM_AUDIO:
			createScope();
			break;
	}

	m_type = type;

	wxClientDC clientDC(this);
	show(clientDC);
}

void CSpectrumDisplay::setSpeed(SPECTRUMSPEED speed)
{
	if (speed == m_speed)
		return;

	switch (speed) {
		case SPECTRUM_200MS:
			m_factor = 1;
			break;
		case SPECTRUM_400MS:
			m_factor = 2;
			break;
		case SPECTRUM_600MS:
			m_factor = 3;
			break;
		case SPECTRUM_800MS:
			m_factor = 4;
			break;
		case SPECTRUM_1000MS:
			m_factor = 5;
			break;
	}

	m_speed = speed;
}

void CSpectrumDisplay::setDB(SPECTRUMRANGE db)
{
	if (db == m_db || m_type == SPECTRUM_PHASE || m_type == SPECTRUM_AUDIO)
		return;

	switch (db) {
		case SPECTRUM_10DB:
			m_dbScale = 10.0F;
			break;
		case SPECTRUM_20DB:
			m_dbScale = 20.0F;
			break;
		case SPECTRUM_30DB:
			m_dbScale = 30.0F;
			break;
		case SPECTRUM_40DB:
			m_dbScale = 40.0F;
			break;
		case SPECTRUM_50DB:
			m_dbScale = 50.0F;
			break;
		case SPECTRUM_60DB:
			m_dbScale = 60.0F;
			break;
	}

	m_db = db;

	switch (m_type) {
		case SPECTRUM_PANADAPTER1:
		case SPECTRUM_PANADAPTER2:
			createPanadapter();
			break;
		default:
			return;
	}

	wxClientDC clientDC(this);
	show(clientDC);
}

SPECTRUMTYPE CSpectrumDisplay::getType() const
{
	return m_type;
}

SPECTRUMSPEED CSpectrumDisplay::getSpeed() const
{
	return m_speed;
}

SPECTRUMRANGE CSpectrumDisplay::getDB() const
{
	return m_db;
}

float CSpectrumDisplay::getFreqPick()
{
   float pick = m_pick;

   m_pick = 0.0F;

   return pick;
}
