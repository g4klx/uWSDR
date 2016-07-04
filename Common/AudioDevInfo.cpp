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

#include "AudioDevInfo.h"
#include "Common.h"

#if defined(USE_PORTAUDIO)
#include "portaudio.h"
#endif


CAudioDevInfo::CAudioDevInfo() :
m_apis(),
m_devs()
{
}

CAudioDevInfo::~CAudioDevInfo()
{
	freeAPIs();
	freeDevs();
}

bool CAudioDevInfo::enumerateAPIs()
{
	freeAPIs();
	freeDevs();

#if defined(USE_JACK)
	CAudioDevAPI* api = new CAudioDevAPI(JACK_API, wxT("Jack"), true, JACK_DEV, JACK_DEV, SOUND_JACK);
	m_apis.push_back(api);
#endif

#if defined(USE_PORTAUDIO)
	PaError error = ::Pa_Initialize();
	if (error != paNoError)
		return false;

	PaHostApiIndex n = ::Pa_GetHostApiCount();

	if (n <= 0) {
		::Pa_Terminate();
		return false;
	}

	PaHostApiIndex defAPI = ::Pa_GetDefaultHostApi();

	for (PaHostApiIndex i = 0; i < n; i++) {
		const PaHostApiInfo* hostAPI = ::Pa_GetHostApiInfo(i);

		wxString name(hostAPI->name, wxConvLocal);

		CAudioDevAPI* api = new CAudioDevAPI(i, name, i == defAPI, hostAPI->defaultInputDevice, hostAPI->defaultOutputDevice, SOUND_PORTAUDIO);
		m_apis.push_back(api);
	}

	::Pa_Terminate();
#endif

	return true;
}

bool CAudioDevInfo::enumerateDevs()
{
	freeDevs();

#if defined(USE_JACK)
	CAudioDevDev* dev = new CAudioDevDev(wxT("Jack"), SOUND_JACK);
	dev->setIn(JACK_API, JACK_DEV, 2U);
	dev->setOut(JACK_API, JACK_DEV, 2U);
	m_devs.push_back(dev);
#endif

#if defined(USE_PORTAUDIO)
	PaError error = ::Pa_Initialize();
	if (error != paNoError)
		return false;

	PaDeviceIndex n = ::Pa_GetDeviceCount();

	if (n <= 0) {
		::Pa_Terminate();
		return false;
	}

	for (PaDeviceIndex i = 0; i < n; i++) {
		const PaDeviceInfo* device = ::Pa_GetDeviceInfo(i);

		wxString name(device->name, wxConvLocal);

		if (device->maxInputChannels > 0) {
			CAudioDevDev* dev = findDev(name, SOUND_PORTAUDIO);

			if (dev == NULL) {
				dev = new CAudioDevDev(name, SOUND_PORTAUDIO);
				m_devs.push_back(dev);
			}

			dev->setIn(device->hostApi, i, device->maxInputChannels);
		}

		if (device->maxOutputChannels > 0) {
			CAudioDevDev* dev = findDev(name, SOUND_PORTAUDIO);

			if (dev == NULL) {
				dev = new CAudioDevDev(name, SOUND_PORTAUDIO);
				m_devs.push_back(dev);
			}

			dev->setOut(device->hostApi, i, device->maxOutputChannels);
		}
	}

	::Pa_Terminate();
#endif

	return true;
}

vector<CAudioDevAPI*>& CAudioDevInfo::getAPIs()
{
	return m_apis;
}

vector<CAudioDevDev*>& CAudioDevInfo::getDevs()
{
	return m_devs;
}

CAudioDevAPI* CAudioDevInfo::findAPI(const wxString& name)
{
	for (unsigned int i = 0; i < m_apis.size(); i++) {
		CAudioDevAPI* api = m_apis.at(i);

		if (api->getName().IsSameAs(name))
			return api;
	}

	return NULL;
}

CAudioDevDev* CAudioDevInfo::findDev(const wxString& name, SOUNDTYPE type)
{
	for (unsigned int i = 0; i < m_devs.size(); i++) {
		CAudioDevDev* dev = m_devs.at(i);

		if (dev->getName().IsSameAs(name) && dev->getType() == type)
			return dev;
	}

	return NULL;
}

void CAudioDevInfo::freeAPIs()
{
	for (unsigned int i = 0; i < m_apis.size(); i++)
		delete m_apis.at(i);

	m_apis.clear();
}

void CAudioDevInfo::freeDevs()
{
	for (unsigned int i = 0; i < m_devs.size(); i++)
		delete m_devs.at(i);

	m_devs.clear();
}
