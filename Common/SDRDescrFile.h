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

#ifndef	SDRDescrFile_H
#define	SDRDescrFile_H

#include <wx/wx.h>

#include "FrequencyRangeArray.h"
#include "Frequency.h"

enum SDRTYPE {
	TYPE_DEMO,
	TYPE_HACKRF,
	TYPE_SI570RX,
	TYPE_SI570TXRX
};

class CSDRDescrFile {
public:
	CSDRDescrFile(const wxString& fileName);
	~CSDRDescrFile();

	wxString             getName() const;
	SDRTYPE              getType() const;
	CFrequency           getMaxFreq() const;
	CFrequency           getMinFreq() const;
	CFrequencyRangeArray getTXRanges() const;
	unsigned int         getFreqMult() const;
	CFrequency           getOffset() const;
	unsigned int         getStepSize() const;
	float                getSampleRate() const;
	bool                 getSwapIQ() const;
	unsigned int         getReceiveGainOffset() const;
	bool                 isValid() const;

private:
	wxString             m_name;
	SDRTYPE              m_type;
	CFrequency           m_maxFreq;
	CFrequency           m_minFreq;
	CFrequencyRangeArray m_ranges;
	unsigned int         m_freqMult;
	CFrequency           m_offset;
	unsigned int         m_stepSize;
	float                m_sampleRate;
	bool                 m_swapIQ;
	unsigned int         m_receiveGainOffset;
	bool                 m_valid;
};

#endif
