/*
 *   Copyright (C) 2006,2007.2013 by Jonathan Naylor G4KLX
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

#ifndef	SoundCardDialog_H
#define	SoundCardDialog_H

#include <wx/wx.h>

#include "AudioDevInfo.h"

const int NO_API = -2;
const int NO_DEV = -2;

class CSoundCardDialog : public wxDialog {

    public:
	CSoundCardDialog(wxWindow* parent, const wxString& title, SOUNDTYPE type, int inDev, int outDev, unsigned int minIn, unsigned int minOut, int id = -1);
	virtual ~CSoundCardDialog();

	void onAPI(wxCommandEvent& event);
	void onOK(wxCommandEvent& event);

	virtual SOUNDTYPE getType() const;
	virtual int       getInDev() const;
	virtual int       getOutDev() const;

    private:
	wxChoice*     m_apiChoice;
	wxChoice*     m_inDevChoice;
	wxChoice*     m_outDevChoice;
	CAudioDevInfo m_info;
	SOUNDTYPE     m_type;
	int           m_inDev;
	int           m_outDev;
	unsigned int  m_minIn;
	unsigned int  m_minOut;

	DECLARE_EVENT_TABLE()

	void enumerateAPI();
	void enumerateAudio(const CAudioDevAPI& api);
};

#endif
