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

#ifndef	AudioDevInfo_H
#define	AudioDevInfo_H

#include <wx/wx.h>

#include <vector>
using namespace std;

enum SOUNDTYPE {
	SOUND_JACK,
	SOUND_PORTAUDIO
};

class CAudioDevAPI {
    public:
	CAudioDevAPI(int api, const wxString& name, bool def, int inDef, int outDef, SOUNDTYPE type) :
	m_api(api),
	m_name(name),
	m_def(def),
	m_inDef(inDef),
	m_outDef(outDef),
	m_type(type)
	{
	}

	~CAudioDevAPI()
	{
	}

	int getAPI() const
	{
		return m_api;
	}

	wxString getName() const
	{
		return m_name;
	}

	bool getDefault() const
	{
		return m_def;
	}

	int getInDefault() const
	{
		return m_inDef;
	}

	int getOutDefault() const
	{
		return m_outDef;
	}

	SOUNDTYPE getType() const
	{
		return m_type;
	}

    private:
	int       m_api;
	wxString  m_name;
	bool      m_def;
	int       m_inDef;
	int       m_outDef;
	SOUNDTYPE m_type;
};

class CAudioDevDev {
    public:
	CAudioDevDev(const wxString& name, SOUNDTYPE type) :
	m_name(name),
	m_type(type),
	m_api(-1),
	m_inDev(-1),
	m_outDev(-1),
	m_inChannels(0),
	m_outChannels(0)
	{
	}

	~CAudioDevDev()
	{
	}

	wxString getName() const
	{
		return m_name;
	}

	SOUNDTYPE getType() const
	{
		return m_type;
	}

	int getAPI() const
	{
		return m_api;
	}

	int getInDev() const
	{
		return m_inDev;
	}

	int getOutDev() const
	{
		return m_outDev;
	}

	int getInChannels() const
	{
		return m_inChannels;
	}

	int getOutChannels() const
	{
		return m_outChannels;
	}

	void setIn(int api, int dev, int channels)
	{
		m_api        = api;
		m_inDev      = dev;
		m_inChannels = channels;
	}

	void setOut(int api, int dev, int channels)
	{
		m_api         = api;
		m_outDev      = dev;
		m_outChannels = channels;
	}

    private:
	wxString  m_name;
	SOUNDTYPE m_type;
	int       m_api;
	int       m_inDev;
	int       m_outDev;
	int       m_inChannels;
	int       m_outChannels;
};

class CAudioDevInfo {
    public:
	CAudioDevInfo();
	virtual ~CAudioDevInfo();

	bool enumerateAPIs();
	bool enumerateDevs();

	vector<CAudioDevAPI*>& getAPIs();
	vector<CAudioDevDev*>& getDevs();

	CAudioDevAPI* findAPI(const wxString& name);
	CAudioDevDev* findDev(const wxString& name, SOUNDTYPE type);

    private:
	vector<CAudioDevAPI*> m_apis;
	vector<CAudioDevDev*> m_devs;

	void freeAPIs();
	void freeDevs();
};

#endif
