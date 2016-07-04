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

#include "FreqDisplay.h"

#include "UWSDRDefs.h"

BEGIN_EVENT_TABLE(CFreqDisplay, wxPanel)
	EVT_PAINT(CFreqDisplay::onPaint)
END_EVENT_TABLE()

CFreqDisplay::CFreqDisplay(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) :
wxPanel(parent, id, pos, size, style, name),
m_width(size.GetWidth()),
m_height(size.GetHeight()),
m_bitmap(NULL),
m_lastFrequency(),
m_lightColour(wxColour(0, 255, 255)),
m_mhzDigits(0)
{
	m_bitmap = new wxBitmap(m_width, m_height);

	// Flood the graph area with black to start with
	clearGraph();
}

CFreqDisplay::~CFreqDisplay()
{
	delete m_bitmap;
}

void CFreqDisplay::setMaxFrequency(const CFrequency& frequency)
{
	wxInt64 hz = frequency.get();

	m_mhzDigits = 1;

	if (hz >= 10000000000LL)		// 10 GHz
		m_mhzDigits = 5;
	else if (hz >= 1000000000LL)		// 1000 MHz
		m_mhzDigits = 4;
	else if (hz >= 100000000LL)		// 100 MHz
		m_mhzDigits = 3;
	else if (hz >= 10000000LL)		// 10 MHz
		m_mhzDigits = 2;
}

void CFreqDisplay::setFrequency(const CFrequency& frequency)
{
	wxInt64 hz = frequency.get();

	// Only display to 10 Hz
	if ((m_lastFrequency.get() / 10LL) == (hz / 10LL))
		return;

	clearGraph();

	wxMemoryDC memoryDC;
	memoryDC.SelectObject(*m_bitmap);

	int mhzDigits = m_mhzDigits;
	if (mhzDigits == 0) {
		mhzDigits = 1;
		if (hz >= 10000000000LL)		// 10 GHz
			mhzDigits = 5;
		else if (hz >= 1000000000LL)		// 1000 MHz
			mhzDigits = 4;
		else if (hz >= 100000000LL)		// 100 MHz
			mhzDigits = 3;
		else if (hz >= 10000000LL)		// 10 MHz
			mhzDigits = 2;
	}

	const int bigThickness    = 5;
	const int littleThickness = 4;

	int bigHeight    = m_height - 2 * BORDER_SIZE;
	int littleHeight = 3 * bigHeight / 4;

	int bigWidth    = (m_width - 2 * BORDER_SIZE) / (mhzDigits + 5);
	int littleWidth = 3 * bigWidth / 4;

	int bigY    = BORDER_SIZE;
	int littleY = (bigHeight + BORDER_SIZE) - littleHeight;

	int x = BORDER_SIZE + (mhzDigits + 4) * bigWidth;

	wxInt64 rem = hz / 10LL;

	drawDigit(memoryDC, littleWidth, littleHeight, littleThickness, x, littleY, rem % 10LL, false);
	x   -= littleWidth;
	rem /= 10LL;

	drawDigit(memoryDC, littleWidth, littleHeight, littleThickness, x, littleY, rem % 10LL, false);
	x   -= bigWidth;
	rem /= 10LL;

	drawDigit(memoryDC, bigWidth, bigHeight, bigThickness, x, bigY, rem % 10LL, false);
	x   -= bigWidth;
	rem /= 10LL;

	drawDigit(memoryDC, bigWidth, bigHeight, bigThickness, x, bigY, rem % 10LL, false);
	x   -= bigWidth;
	rem /= 10LL;

	drawDigit(memoryDC, bigWidth, bigHeight, bigThickness, x, bigY, rem % 10LL, true);
	x   -= bigWidth;

	rem = hz / 1000000LL;

	for (int i = 0; i < mhzDigits; i++) {
		wxInt64 n = rem % 10LL;
		rem /= 10LL;

		if (rem != 0LL || (rem == 0LL && n != 0LL))
			drawDigit(memoryDC, bigWidth, bigHeight, bigThickness, x, bigY, n, false);

		x -= bigWidth;
	}

	memoryDC.SelectObject(wxNullBitmap);

	wxClientDC clientDC(this);
	show(clientDC);

	m_lastFrequency = frequency;
}

void CFreqDisplay::onPaint(wxPaintEvent& WXUNUSED(event))
{
	wxPaintDC dc(this);

	show(dc);
}

void CFreqDisplay::show(wxDC& dc)
{
	dc.DrawBitmap(*m_bitmap, 0, 0, false);
}

void CFreqDisplay::clearGraph()
{
	// Flood the graph area with black to start with
	wxMemoryDC dc;
	dc.SelectObject(*m_bitmap);

	dc.SetBackground(*wxBLACK_BRUSH);
	dc.Clear();
}

void CFreqDisplay::drawDigit(wxDC& dc, int width, int height, int thickness, int x, int y, unsigned int n, bool dot)
{
	wxASSERT(width > 0);
	wxASSERT(height > 0);
	wxASSERT(thickness > 0);
	wxASSERT(x >= 0);
	wxASSERT(y >= 0);
	wxASSERT(n >= 0 && n <= 9);

	int radius = thickness / 2;;

	int topSpace    = 3;
	int bottomSpace = 3 + thickness;

	int leftSpace  = 3 + thickness;
	int rightSpace = 3 + thickness;

	int barWidth  = width - leftSpace - rightSpace;
	int barHeight = (height - topSpace - bottomSpace) / 2;

	const int BODGE_FACTOR = 2;

	dc.SetPen(wxPen(m_lightColour));
	dc.SetBrush(wxBrush(m_lightColour));

	if (dot)
		dc.DrawRoundedRectangle(x + 2, y + topSpace + 2 * barHeight, thickness, thickness, radius);

	if (n == 0 || n == 2 || n == 3 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9)
		dc.DrawRoundedRectangle(x + leftSpace + BODGE_FACTOR, y + topSpace, barWidth, thickness, radius);

	if (n == 0 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9)
		dc.DrawRoundedRectangle(x + leftSpace, y + topSpace + BODGE_FACTOR, thickness, barHeight, radius);

	if (n == 0 || n == 1 || n == 2 || n == 3 || n == 4 || n == 7 || n == 8 || n == 9)
		dc.DrawRoundedRectangle(x + leftSpace + barWidth, y + topSpace + BODGE_FACTOR, thickness, barHeight, radius);

	if (n == 2 || n == 3 || n == 4 || n == 5 || n == 6 || n == 8 || n == 9)
		dc.DrawRoundedRectangle(x + leftSpace + BODGE_FACTOR, y + topSpace + barHeight, barWidth, thickness, radius);

	if (n == 0 || n == 2 || n == 6 || n == 8)
		dc.DrawRoundedRectangle(x + leftSpace, y + topSpace + barHeight + BODGE_FACTOR, thickness, barHeight, radius);

	if (n == 0 || n == 1 || n == 3 || n == 4 || n == 5 || n == 6 || n == 7 || n == 8 || n == 9)
		dc.DrawRoundedRectangle(x + leftSpace + barWidth, y + topSpace + barHeight + BODGE_FACTOR, thickness, barHeight, radius);

	if (n == 0 || n == 2 || n == 3 ||  n == 5 || n == 6 || n == 8 || n == 9)
		dc.DrawRoundedRectangle(x + leftSpace + BODGE_FACTOR, y + topSpace + 2 * barHeight, barWidth, thickness, radius);
}
