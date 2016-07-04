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

#ifndef	SpectrumDisplay_H
#define	SpectrumDisplay_H

#include <wx/wx.h>

#include "UWSDRDefs.h"
#include "Common.h"


class CSpectrumDisplay : public wxPanel {
public:
	CSpectrumDisplay(wxWindow* parent, int id, const wxPoint& pos, const wxSize& size, long style = 0L, const wxString& name = wxPanelNameStr);
	virtual ~CSpectrumDisplay();

	virtual void setSampleRate(float sampleRate);
	virtual void setBandwidth(float hertz);
	virtual void setFilter(FILTERWIDTH filter, UWSDRMODE mode);

	virtual void showSpectrum(const float* spectrum, float bottom = 0.0F, float offset = 0.0F);

	virtual void setType(SPECTRUMTYPE type);
	virtual void setSpeed(SPECTRUMSPEED speed);
	virtual void setDB(SPECTRUMRANGE db);

	virtual SPECTRUMTYPE  getType() const;
	virtual SPECTRUMSPEED getSpeed() const;
	virtual SPECTRUMRANGE getDB() const;

	virtual float getFreqPick();

	void onPaint(wxPaintEvent& event);
	void onLeftMouse(wxMouseEvent& event);
	void onRightMouse(wxMouseEvent& event);
	void onMenu(wxCommandEvent& event);

private:
	int           m_width;
	int           m_height;
	float         m_dbScale;
	wxBitmap*     m_background;
	wxBitmap*     m_bitmap;
	float         m_sampleRate;
	float         m_bandwidth;
	float         m_highFilter;
	float         m_lowFilter;
	wxMenu*       m_menu;
	wxMenu*       m_speedMenu;
	wxMenu*       m_typeMenu;
	wxMenu*       m_dbMenu;
	SPECTRUMTYPE  m_type;
	SPECTRUMSPEED m_speed;
	SPECTRUMRANGE m_db;
	int           m_factor;
	unsigned int  m_ticks;
	float         m_pick;
	float         m_offset;

	DECLARE_EVENT_TABLE()

	void show(wxDC& dc);

	void createPanadapter();
	void createWaterfall();
	void createPhase();
	void createScope();

	void drawPanadapter1(const float* spectrum, float bottom);
	void drawPanadapter2(const float* spectrum, float bottom);
	void drawWaterfall(const float* spectrum, float bottom);
	void drawPhase(const float* spectrum);
	void drawScope(const float* spectrum);
};

#endif
