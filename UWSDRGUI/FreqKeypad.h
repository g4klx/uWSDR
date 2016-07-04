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

#ifndef	FreqKeypad_H
#define	FreqKeypad_H

#include <wx/wx.h>

#include "Frequency.h"

class CFreqKeypad : public wxDialog {

    public:
	CFreqKeypad(wxWindow* parent, int id, const CFrequency& vfo, const CFrequency& minFreq, const CFrequency& maxFres);
	virtual ~CFreqKeypad();

	void onOK(wxCommandEvent& event);
	void onButton(wxCommandEvent& event);

	virtual CFrequency getFrequency() const;

    private:
	CFrequency  m_vfo;
	CFrequency  m_minFreq;
	CFrequency  m_maxFreq;
	CFrequency  m_frequency;
	wxTextCtrl* m_text;

	DECLARE_EVENT_TABLE()
};

#endif
