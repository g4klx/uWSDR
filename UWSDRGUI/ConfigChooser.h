/*
 *   Copyright (C) 2007 by Jonathan Naylor G4KLX
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

#ifndef	ConfigChooser_H
#define	ConfigChooser_H

#include <wx/wx.h>

class CConfigChooser : public wxDialog {

    public:
	CConfigChooser(wxWindow* parent, int id = -1);
	virtual ~CConfigChooser();

	void onOK(wxCommandEvent& event);
	void onCancel(wxCommandEvent& event);

	virtual wxString getName() const;

    private:
	wxChoice* m_configChoice;
	wxString  m_name;

	DECLARE_EVENT_TABLE()

	void enumerateConfigs();
};

#endif
