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

#ifndef	VoiceKeyboard_H
#define	VoiceKeyboard_H

#include <wx/wx.h>
#include <wx/spinctrl.h>

#include "UWSDRDefs.h"


class CVoiceKeyboard : public wxDialog {

    public:
	CVoiceKeyboard(wxWindow* parent, int id);
	virtual ~CVoiceKeyboard();

	void onBrowse(wxCommandEvent& event);
	void onTransmit(wxCommandEvent& event);
	void onAbort(wxCommandEvent& event);
	void onHelp(wxCommandEvent& event);

	virtual void     setDir(const wxString &dir);
	virtual wxString getDir() const;

	virtual void     setFile(unsigned int n, const wxString& fileName);
	virtual wxString getFile(unsigned int n) const;

    private:
	wxComboBox*    m_filename;
	wxRadioButton* m_single;
	wxRadioButton* m_continuous;
	wxString       m_dir;


	DECLARE_EVENT_TABLE()
};

#endif
