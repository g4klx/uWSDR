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

#ifndef	GUISetupFrame_H
#define	GUISetupFrame_H

#include <wx/wx.h>

#include "SDRDescrFile.h"
#include "AudioDevInfo.h"
#include "Common.h"

class CGUISetupFrame : public wxFrame {
public:
	CGUISetupFrame();
	virtual ~CGUISetupFrame();

	void onName(wxCommandEvent& event);
	void onBrowse(wxCommandEvent& event);
	void onUserAudio(wxCommandEvent& event);
	void onSDRAudio(wxCommandEvent& event);
	void onTuning(wxCommandEvent& event);
	void onExternal(wxCommandEvent& event);
	void onCreate(wxCommandEvent& event);

private:
	wxComboBox*    m_name;
	wxTextCtrl*    m_filenameText;
	wxCheckBox*    m_deskTop;
	wxButton*      m_userAudio;
	wxButton*      m_sdrAudio;
	wxString       m_filename;
	SDRTYPE        m_sdrType;
	SOUNDTYPE      m_userAudioType;
	int            m_userAudioInDev;
	int            m_userAudioOutDev;
	SOUNDTYPE      m_sdrAudioType;
	int            m_sdrAudioInDev;
	int            m_sdrAudioOutDev;
	TUNINGHW       m_tuningHW;
	wxString       m_externalName;
	EXTERNALADDRS  m_externalAddr;

	DECLARE_EVENT_TABLE()

	void enumerateConfigs();
	void readConfig(const wxString& name);
#if defined(__WXMSW__)
	void writeDeskTop(const wxString& name, const wxString& instDir);
#elif defined(__WXGTK__)
	bool getDesktopDir(wxString& dir) const;
	void writeDeskTop(const wxString& name, const wxString& instDir);
#endif
};

#endif
