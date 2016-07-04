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

#ifndef	DSPControl_H
#define	DSPControl_H

#include <wx/wx.h>

#include "Common.h"
#include "UWSDRDefs.h"
#include "SoundFileWriter.h"
#include "DTTSPControl.h"
#include "CWKeyer.h"
#include "VoiceKeyer.h"
#include "ExternalInterface.h"
#include "DataReader.h"
#include "DataWriter.h"
#include "RingBuffer.h"
#include "ExternalProtocolHandler.h"


class CDSPControl : public wxThread, public IDataCallback {
public:
	CDSPControl(float sampleRate, unsigned int receiveGainOffset, unsigned int blockSize, bool swapIQ);
	virtual ~CDSPControl();

	virtual void  setTXReader(IDataReader* reader);
	virtual void  setTXWriter(IDataWriter* writer);

	virtual void  setRXReader(IDataReader* reader);
	virtual void  setRXWriter(IDataWriter* writer);

	virtual void  setTXInControl(IExternalInterface* control);
	virtual void  setKeyInControl(IExternalInterface* control);

	virtual void  setKey(bool keyOn);

	virtual bool  open();
	virtual void* Entry();
	virtual void  close();

	virtual bool  openIO();
	virtual void  closeIO();

	virtual void  callback(float* buffer, unsigned int nSamples, int id);

	// Many of these are pass throughs to DTTSP
	virtual void setMode(UWSDRMODE mode);
	virtual void setWeaver(bool onOff);
	virtual void setFilter(FILTERWIDTH filter);
	virtual void setAGC(AGCSPEED agc);
	virtual void setDeviation(FMDEVIATION dev);
	virtual void setTXAndFreq(TXSTATE transmit, float freq);

	virtual void setNB(bool onOff);
	virtual void setNBValue(unsigned int value);
	virtual void setNB2(bool onOff);
	virtual void setNB2Value(unsigned int value);

	virtual void setSP(bool onOff);
	virtual void setSPValue(unsigned int value);

	virtual void setEQ(bool onOff);
	virtual void setEQLevels(unsigned int n, const int* levels);

	virtual void setCarrierLevel(unsigned int value);

	virtual void setALCValue(unsigned int attack, unsigned int decay, unsigned int hang);

	virtual void setRXIAndQ(int phase, int gain);
	virtual void setTXIAndQ(int phase, int gain);

	virtual void setBinaural(bool onOff);
	virtual void setPan(int value);

	virtual bool setRecordOn(RECORDTYPE type);
	virtual void setRecordOff();

	virtual void setAFGain(unsigned int value);
	virtual void setRFGain(unsigned int value);
	virtual void setMicGain(unsigned int value);
	virtual void setPower(unsigned int value);
	virtual void setSquelch(unsigned int value);

	virtual float getMeter(METERPOS type);
	virtual void  getSpectrum(float* spectrum);
	virtual void  getPhase(float* spectrum);
	virtual void  getScope(float* spectrum);

	virtual float getTXOffset();
	virtual float getRXOffset();

	virtual CWERROR    sendCW(unsigned int speed, const wxString& text, CWSTATUS state);
	virtual VOICEERROR sendAudio(const wxString& fileName, VOICESTATUS state);

	virtual void  setExternalHandler(CExternalProtocolHandler* handler);

private:
	CDTTSPControl*  m_dttsp;
	CCWKeyer*       m_cwKeyer;
	CVoiceKeyer*    m_voiceKeyer;

	float           m_sampleRate;
	unsigned int    m_blockSize;

	IDataReader*    m_txReader;
	IDataWriter*    m_txWriter;
	IDataReader*    m_rxReader;
	IDataWriter*    m_rxWriter;

	IExternalInterface* m_txInControl;
	IExternalInterface* m_keyInControl;

	wxSemaphore     m_waiting;

	CRingBuffer     m_txRingBuffer;
	CRingBuffer     m_rxRingBuffer;
	float*          m_txBuffer;
	float*          m_rxBuffer;
	float*          m_outBuffer;
	float*          m_txLastBuffer;
	float*          m_rxLastBuffer;

	CSoundFileWriter* m_record;

	TXSTATE         m_transmit;
	bool            m_running;
	UWSDRMODE       m_mode;
	RECORDTYPE      m_recordType;

	float           m_afGain;

	int             m_clockId;

	bool            m_lastTXIn;
	bool            m_lastKeyIn;

	unsigned int    m_rxOverruns;
	unsigned int    m_rxFills;
	unsigned int    m_txOverruns;
	unsigned int    m_txFills;

	CExternalProtocolHandler* m_extHandler;

#if defined(__WXDEBUG__)
	void dumpBuffer(const wxString& title, float* buffer, unsigned int nSamples) const;
#endif
};

#endif
