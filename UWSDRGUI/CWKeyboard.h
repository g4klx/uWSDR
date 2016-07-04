/*
 *   Copyright (C) 2006-2007,2013 by Jonathan Naylor G4KLX
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

#ifndef	CWKeyboard_H
#define	CWKeyboard_H

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "UWSDRDefs.h"

class CCWKeyboard : public wxDialog {

    public:
	CCWKeyboard(wxWindow* parent, int id);
	virtual ~CCWKeyboard();

	void onRealTime(wxCommandEvent& event);
	void onTransmit(wxCommandEvent& event);
	void onAbort(wxCommandEvent& event);
	void onHelp(wxCommandEvent& event);

	virtual void     setLocal(const wxString& text);
	virtual wxString getLocal() const;

	virtual void     setRemote(const wxString& text);
	virtual wxString getRemote() const;

	virtual void     setLocator(const wxString& text);
	virtual wxString getLocator() const;

	virtual void     setReport(const wxString& text);
	virtual wxString getReport() const;

	virtual void     setSerial(const wxString& text);
	virtual wxString getSerial() const;

	virtual void     setMessage(unsigned int n, const wxString& text);
	virtual wxString getMessage(unsigned int n) const;

	virtual void         setSpeed(unsigned int speed);
	virtual unsigned int getSpeed() const;

    private:
	wxTextCtrl*     m_local;
	wxTextCtrl*     m_remote;
	wxTextCtrl*     m_locator;
	wxTextCtrl*     m_report;
	wxTextCtrl*     m_serial;
	wxTextCtrl**    m_text;
	wxRadioButton** m_button;
	wxTextCtrl*     m_realTime;
	wxSpinCtrl*     m_speed;
	wxButton*       m_abortButton;
	unsigned int    m_prevLen;

	DECLARE_EVENT_TABLE()
};

#endif
