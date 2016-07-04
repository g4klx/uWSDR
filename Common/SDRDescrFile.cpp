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

#include "SDRDescrFile.h"

#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/file.h>

CSDRDescrFile::CSDRDescrFile(const wxString& fileName) :
m_name(),
m_type(TYPE_DEMO),
m_maxFreq(),
m_minFreq(),
m_ranges(),
m_freqMult(1U),
m_offset(),
m_stepSize(0U),
m_sampleRate(0.0F),
m_swapIQ(false),
m_receiveGainOffset(0U),
m_valid(false)
{
	if (!wxFile::Exists(fileName))
		return;

	wxTextFile file;

	bool ret = file.Open(fileName);
	if (!ret)
		return;

	unsigned int nLines = file.GetLineCount();

	for (unsigned int i = 0; i < nLines; i++) {
		wxString line = file.GetLine(i);

		if (line.length() > 0 && line.GetChar(0) != wxT('#')) {
			if (line.Left(5).Cmp(wxT("name=")) == 0) {
				m_name = line.Mid(5);
			} else if (line.Left(5).Cmp(wxT("type=")) == 0) {
				wxString type = line.Mid(5);

				if (type.Cmp(wxT("demo")) == 0)
					m_type = TYPE_DEMO;
				else if (type.Cmp(wxT("hackrf")) == 0)
					m_type = TYPE_HACKRF;
				else if (type.Cmp(wxT("si570rx")) == 0)
					m_type = TYPE_SI570RX;
				else if (type.Cmp(wxT("si570txrx")) == 0)
					m_type = TYPE_SI570TXRX;
				else {
					wxLogError(wxT("Unknown type - %s in the .sdr file"), type.c_str());
					return;
				}
			} else if (line.Left(15).Cmp(wxT("frequencyRange=")) == 0) {
				wxString freqs = line.Mid(15);
				int pos = freqs.Find(wxT(','));
				if (pos != wxNOT_FOUND) {
					m_minFreq.set(freqs.Left(pos));
					m_maxFreq.set(freqs.Mid(pos + 1));
				} else {
					wxLogError(wxT("The frequencyRange entry is invalid - \"%s\""), line.c_str());
					return;
				}
			} else if (line.Left(13).Cmp(wxT("transmitBand=")) == 0) {
				wxString freqs = line.Mid(13);
				int pos = freqs.Find(wxT(','));
				if (pos != wxNOT_FOUND) {
					m_ranges.addRange(freqs.Left(pos), freqs.Mid(pos + 1));
				} else {
					wxLogError(wxT("A transmitBand entry is invalid - \"%s\""), line.c_str());
					return;
				}
			} else if (line.Left(20).Cmp(wxT("frequencyMultiplier=")) == 0) {
				unsigned long temp;
				line.Mid(20).ToULong(&temp);
				m_freqMult = temp;
			} else if (line.Left(16).Cmp(wxT("frequencyOffset=")) == 0) {
				m_offset.set(line.Mid(16));
			} else if (line.Left(9).Cmp(wxT("stepSize=")) == 0) {
				unsigned long temp;
				line.Mid(9).ToULong(&temp);
				m_stepSize = temp;
			} else if (line.Left(11).Cmp(wxT("sampleRate=")) == 0) {
				double temp;
				line.Mid(11).ToDouble(&temp);
				m_sampleRate = temp;
			} else if (line.Left(7).Cmp(wxT("swapIQ=")) == 0) {
				unsigned long temp;
				line.Mid(7).ToULong(&temp);
				m_swapIQ = temp == 1L;
			} else if (line.Left(18).Cmp(wxT("receiveGainOffset=")) == 0) {
				unsigned long temp;
				line.Mid(18).ToULong(&temp);
				m_receiveGainOffset = temp;
			}
		}
	}

	m_valid = true;

	file.Close();
}

CSDRDescrFile::~CSDRDescrFile()
{
}

wxString CSDRDescrFile::getName() const
{
	return m_name;
}

SDRTYPE CSDRDescrFile::getType() const
{
	return m_type;
}

CFrequency CSDRDescrFile::getMaxFreq() const
{
	return m_maxFreq;
}

CFrequency CSDRDescrFile::getMinFreq() const
{
	return m_minFreq;
}

CFrequencyRangeArray CSDRDescrFile::getTXRanges() const
{
	return m_ranges;
}

unsigned int CSDRDescrFile::getFreqMult() const
{
	return m_freqMult;
}

CFrequency CSDRDescrFile::getOffset() const
{
	return m_offset;
}

unsigned int CSDRDescrFile::getStepSize() const
{
	return m_stepSize;
}

float CSDRDescrFile::getSampleRate() const
{
	return m_sampleRate;
}

bool CSDRDescrFile::getSwapIQ() const
{
	return m_swapIQ;
}

bool CSDRDescrFile::isValid() const
{
	return m_valid;
}

unsigned int CSDRDescrFile::getReceiveGainOffset() const
{
	return m_receiveGainOffset;
}
