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

#ifndef	UWSDRPreferences_H
#define	UWSDRPreferences_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/notebook.h>

#include "SDRParameters.h"
#include "DSPControl.h"


class CUWSDRPreferences : public wxDialog {
public:
	CUWSDRPreferences(wxWindow* parent, int id, CSDRParameters* parameters, CDSPControl* dsp);
	virtual ~CUWSDRPreferences();

	void onIQChanged(wxSpinEvent& event);
	void onRFGainChanged(wxScrollEvent& event);
	void onTXEQEnabled(wxCommandEvent& event);
	void onTXEQChanged(wxScrollEvent& event);

	void onOK(wxCommandEvent& event);
	void onHelp(wxCommandEvent& event);

private:
	CSDRParameters* m_parameters;
	CDSPControl*    m_dsp;
	wxNotebook*     m_noteBook;
	wxTextCtrl*     m_freqShift1;
	wxTextCtrl*     m_freqShift2;
	wxTextCtrl*     m_freqOffset;
	wxChoice*       m_deviationFMW;
	wxChoice*       m_deviationFMN;
	wxChoice*       m_agcAM;
	wxChoice*       m_agcSSB;
	wxChoice*       m_agcCW;
	wxChoice*       m_agcDig;
	wxChoice*       m_filterFMW;
	wxChoice*       m_filterFMN;
	wxChoice*       m_filterAM;
	wxChoice*       m_filterSSB;
	wxChoice*       m_filterCWW;
	wxChoice*       m_filterCWN;
	wxChoice*       m_filterDig;
	wxChoice*       m_tuningFM;
	wxChoice*       m_tuningAM;
	wxChoice*       m_tuningSSB;
	wxChoice*       m_tuningCWW;
	wxChoice*       m_tuningCWN;
	wxChoice*       m_tuningDig;
	wxTextCtrl*     m_stepVeryFast;
	wxTextCtrl*     m_stepFast;
	wxTextCtrl*     m_stepMedium;
	wxTextCtrl*     m_stepSlow;
	wxTextCtrl*     m_stepVerySlow;
	wxCheckBox*     m_nbButton;
	wxSlider*       m_nbValue;
	wxCheckBox*     m_nb2Button;
	wxSlider*       m_nb2Value;
	wxCheckBox*     m_spButton;
	wxSlider*       m_spValue;
	wxCheckBox*     m_binButton;
	wxSlider*       m_panValue;
	wxChoice*       m_recordType;
	wxSlider*       m_carrierLevel;
	wxSlider*       m_alcAttack;
	wxSlider*       m_alcDecay;
	wxSlider*       m_alcHang;
	wxSlider*       m_rfValue;
	wxSpinCtrl*     m_rxIQPhase;
	wxSpinCtrl*     m_rxIQGain;
	wxSpinCtrl*     m_txIQPhase;
	wxSpinCtrl*     m_txIQGain;
	wxChoice*       m_method;
	wxChoice*       m_txEqEnable;
	wxSlider*       m_txEqPreamp;
	wxSlider*       m_txEqGain0;
	wxSlider*       m_txEqGain1;
	wxSlider*       m_txEqGain2;

	DECLARE_EVENT_TABLE()

	wxPanel* createFrequencyTab(wxNotebook* noteBook);
	wxPanel* createModeTab(wxNotebook* noteBook);
	wxPanel* createStepTab(wxNotebook* noteBook);
	wxPanel* createReceiveTab(wxNotebook* noteBook);
	wxPanel* createTransmitTab(wxNotebook* noteBook);
	wxPanel* createALCTab(wxNotebook* noteBook);
	wxPanel* createIQTab(wxNotebook* noteBook);
	wxPanel* createTXEQTab(wxNotebook* noteBook);

	wxChoice* createDeviationChoice(wxPanel* panel);
	wxChoice* createAGCChoice(wxPanel* panel);
	wxChoice* createFilterChoice(wxPanel* panel);
	wxChoice* createTuningChoice(wxPanel* panel);
};

#endif
