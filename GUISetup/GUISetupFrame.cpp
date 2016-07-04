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

#include "GUISetupFrame.h"

#include "SoundCardDialog.h"
#include "TuningDialog.h"
#include "ExternalDialog.h"

#include <wx/config.h>
#include <wx/filename.h>
#include <wx/dir.h>

#if defined(__WXGTK__) || defined(__WXMAC__)
#include "GUISetup.xpm"
#endif

#if defined(__WXGTK__)
const wxChar* HOME_ENV      = wxT("HOME");
const wxChar* DESKTOP_DIR   = wxT("/Desktop");
const wxChar* SKELETON_FILE = wxT("Skeleton.desktop");
const wxChar* NAME_TOKEN    = wxT("@NAME@");
#endif

#if defined(__WINDOWS__)
#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#endif

const struct {
	bool sdrAudioButton;
	bool txOutData;
	bool audioCheck;
} featureList[] = {
	{false, false, false},	// TYPE_DEMO
	{false, false, false},	// TYPE_HACKRF
	{true,  false, false},	// TYPE_SI570RX
	{true,  true,  true}	// TYPE_SI570TXRX
};

const int CREATE_BUTTON     = 27543;
const int BROWSE_BUTTON     = 27544;
const int USER_AUDIO_BUTTON = 27545;
const int SDR_AUDIO_BUTTON  = 27546;
const int TUNING_BUTTON     = 27550;
const int NAME_COMBO        = 27551;
const int EXTERNAL_BUTTON   = 27552;


const int BORDER_SIZE     = 5;
const int DATA_WIDTH      = 150;


BEGIN_EVENT_TABLE(CGUISetupFrame, wxFrame)
	EVT_COMBOBOX(NAME_COMBO,      CGUISetupFrame::onName)
	EVT_TEXT_ENTER(NAME_COMBO,    CGUISetupFrame::onName)
	EVT_BUTTON(CREATE_BUTTON,     CGUISetupFrame::onCreate)
	EVT_BUTTON(BROWSE_BUTTON,     CGUISetupFrame::onBrowse)
	EVT_BUTTON(USER_AUDIO_BUTTON, CGUISetupFrame::onUserAudio)
	EVT_BUTTON(SDR_AUDIO_BUTTON,  CGUISetupFrame::onSDRAudio)
	EVT_BUTTON(TUNING_BUTTON,     CGUISetupFrame::onTuning)
	EVT_BUTTON(EXTERNAL_BUTTON,   CGUISetupFrame::onExternal)
END_EVENT_TABLE()


CGUISetupFrame::CGUISetupFrame() :
wxFrame(NULL, -1, wxString(_("UWSDR GUI Setup")), wxDefaultPosition, wxDefaultSize, wxMINIMIZE_BOX  | wxSYSTEM_MENU | wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN),
m_name(NULL),
m_filenameText(NULL),
m_deskTop(NULL),
m_userAudio(NULL),
m_sdrAudio(NULL),
m_filename(),
m_sdrType(TYPE_DEMO),
m_userAudioType(),
m_userAudioInDev(NO_DEV),
m_userAudioOutDev(NO_DEV),
m_sdrAudioType(),
m_sdrAudioInDev(NO_DEV),
m_sdrAudioOutDev(NO_DEV),
m_tuningHW(TUNINGHW_NONE),
m_externalName(),
m_externalAddr(EXTERNALADDRS_LOCAL)
{
	SetIcon(wxICON(GUISetup));

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

	wxPanel* panel = new wxPanel(this, -1);

	wxFlexGridSizer* panelSizer = new wxFlexGridSizer(3);

	wxStaticText* label1 = new wxStaticText(panel, -1, _("Name:"));
	panelSizer->Add(label1, 0, wxALL, BORDER_SIZE);

	m_name = new wxComboBox(panel, NAME_COMBO, wxEmptyString, wxDefaultPosition, wxSize(DATA_WIDTH, -1), 0, NULL, wxCB_DROPDOWN);
	panelSizer->Add(m_name, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy1 = new wxStaticText(panel, -1, wxEmptyString);
	panelSizer->Add(dummy1, 0, wxALL, BORDER_SIZE);

	wxStaticText* label2 = new wxStaticText(panel, -1, _("SDR File Name:"));
	panelSizer->Add(label2, 0, wxALL, BORDER_SIZE);

	m_filenameText = new wxTextCtrl(panel, -1, wxEmptyString, wxDefaultPosition, wxSize(DATA_WIDTH, -1), wxTE_READONLY);
	panelSizer->Add(m_filenameText, 0, wxALL, BORDER_SIZE);

	wxButton* browse = new wxButton(panel, BROWSE_BUTTON, _("Browse..."));
	panelSizer->Add(browse, 0, wxALL, BORDER_SIZE);

	wxStaticText* label3 = new wxStaticText(panel, -1, _("User Audio:"));
	panelSizer->Add(label3, 0, wxALL, BORDER_SIZE);

	m_userAudio = new wxButton(panel, USER_AUDIO_BUTTON, _("Set"), wxDefaultPosition, wxSize(DATA_WIDTH, -1));
	panelSizer->Add(m_userAudio, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy2 = new wxStaticText(panel, -1, wxEmptyString);
	panelSizer->Add(dummy2, 0, wxALL, BORDER_SIZE);

	wxStaticText* label4 = new wxStaticText(panel, -1, _("SDR Audio:"));
	panelSizer->Add(label4, 0, wxALL, BORDER_SIZE);

	m_sdrAudio = new wxButton(panel, SDR_AUDIO_BUTTON, _("Set"), wxDefaultPosition, wxSize(DATA_WIDTH, -1));
	m_sdrAudio->Disable();
	panelSizer->Add(m_sdrAudio, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy3 = new wxStaticText(panel, -1, wxEmptyString);
	panelSizer->Add(dummy3, 0, wxALL, BORDER_SIZE);

	wxStaticText* label8 = new wxStaticText(panel, -1, _("Tuning Hardware:"));
	panelSizer->Add(label8, 0, wxALL, BORDER_SIZE);

	wxButton* tuning = new wxButton(panel, TUNING_BUTTON, _("Set"), wxDefaultPosition, wxSize(DATA_WIDTH, -1));
	panelSizer->Add(tuning, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy7 = new wxStaticText(panel, -1, wxEmptyString);
	panelSizer->Add(dummy7, 0, wxALL, BORDER_SIZE);

	wxStaticText* label9 = new wxStaticText(panel, -1, _("External Programs:"));
	panelSizer->Add(label9, 0, wxALL, BORDER_SIZE);

	wxButton* external = new wxButton(panel, EXTERNAL_BUTTON, _("Set"), wxDefaultPosition, wxSize(DATA_WIDTH, -1));
	panelSizer->Add(external, 0, wxALL, BORDER_SIZE);

	wxStaticText* dummy8 = new wxStaticText(panel, -1, wxEmptyString);
	panelSizer->Add(dummy8, 0, wxALL, BORDER_SIZE);

#if !defined(__WXMAC__)
#if defined(__WXGTK__)
	wxString dir;
	if (getDesktopDir(dir)) {
#endif
		wxStaticText* label10 = new wxStaticText(panel, -1, _("Create Desktop icon:"));
		panelSizer->Add(label10, 0, wxALL, BORDER_SIZE);

		m_deskTop = new wxCheckBox(panel, -1, wxEmptyString);
		panelSizer->Add(m_deskTop, 0, wxALL, BORDER_SIZE);

		wxStaticText* dummy9 = new wxStaticText(panel, -1, wxEmptyString);
		panelSizer->Add(dummy9, 0, wxALL, BORDER_SIZE);
#if defined(__WXGTK__)
	}
#endif
#endif

	wxStaticText* dummy10 = new wxStaticText(panel, -1, wxEmptyString);
	panelSizer->Add(dummy10, 0, wxALL, BORDER_SIZE);

	wxButton* create = new wxButton(panel, CREATE_BUTTON, _("Create"), wxDefaultPosition, wxSize(DATA_WIDTH, -1));
	panelSizer->Add(create, 0, wxALL, BORDER_SIZE);

	panel->SetSizer(panelSizer);

	mainSizer->Add(panel);

	SetSizer(mainSizer);

	mainSizer->SetSizeHints(this);

	enumerateConfigs();
}

CGUISetupFrame::~CGUISetupFrame()
{
}

void CGUISetupFrame::onName(wxCommandEvent& WXUNUSED(event))
{
	wxString name = m_name->GetValue();

	readConfig(name);
}

void CGUISetupFrame::onBrowse(wxCommandEvent& WXUNUSED(event))
{
	// Pre-load the directory where the .sdr files are located
#if defined(__WXMSW__)
	wxConfig* config = new wxConfig(wxT("UWSDR"));

	wxString instDirKey = wxT("/InstPath");

	wxString sdrDir;
	bool found = config->Read(instDirKey, &sdrDir);

	delete config;

	if (!found) {
		::wxMessageBox(_("Cannot find the registry key for the\ninstallation directory."), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	sdrDir.Append(wxT("\\SDR Files"));
#else
	wxString sdrDir = wxT(DATA_DIR);
#endif

	wxFileDialog files(this, _("Choose an SDR Configuration File"), wxEmptyString, wxEmptyString, _("SDR files (*.sdr)|*.sdr"), wxFD_FILE_MUST_EXIST);
	files.SetDirectory(sdrDir);

	int ret = files.ShowModal();

	if (ret != wxID_OK)
		return;

	m_filename = files.GetPath();

	wxFileName filePath(m_filename);

	m_filenameText->SetValue(filePath.GetFullName());

	// Clear everything
	m_sdrAudio->Disable();

	CSDRDescrFile file(m_filename);
	if (!file.isValid()) {
		::wxMessageBox(_("Cannot open the SDR File"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	m_sdrType = file.getType();

	if (featureList[m_sdrType].sdrAudioButton)
		m_sdrAudio->Enable();
}

void CGUISetupFrame::onCreate(wxCommandEvent& WXUNUSED(event))
{
	wxString name = m_name->GetValue();
	if (name.IsEmpty()) {
		::wxMessageBox(_("The Name is not allowed to be empty"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	// Whitespace isn't allowed in the SDR name
	int pos1 = name.Find(wxT(' '));
	int pos2 = name.Find(wxT('\t'));
	if (pos1 != wxNOT_FOUND || pos2 != wxNOT_FOUND) {
		::wxMessageBox(_("The Name cannot contain white space (spaces, tabs, etc)"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	if (m_filename.IsEmpty()) {
		::wxMessageBox(_("The SDR File Name is not allowed to be empty"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	CSDRDescrFile file(m_filename);
	if (!file.isValid()) {
		::wxMessageBox(_("The SDR File does not exist or is invalid"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	m_sdrType = file.getType();

	if (m_userAudioInDev == NO_DEV || m_userAudioOutDev == NO_DEV) {
		::wxMessageBox(_("The User Audio has not been set"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	if (featureList[m_sdrType].sdrAudioButton) {
		if (m_sdrAudioInDev == NO_DEV || m_sdrAudioOutDev == NO_DEV) {
			::wxMessageBox(_("The SDR Audio has not been set"), _("GUISetup Error"), wxICON_ERROR);
			return;
		}
	}

	if (featureList[m_sdrType].audioCheck) {
		if (m_sdrAudioType != SOUND_JACK && m_userAudioType == m_sdrAudioType && (m_sdrAudioInDev == m_userAudioInDev || m_sdrAudioOutDev == m_userAudioOutDev)) {
			::wxMessageBox(_("The SDR Audio cannot be the same as the User Audio"), _("GUISetup Error"), wxICON_ERROR);
			return;
		}
	}

	wxConfig* config = new wxConfig(wxT("UWSDR"));

	wxString fileNameKey        = wxT("/") + name + wxT("/FileName");
	wxString userAudioTypeKey   = wxT("/") + name + wxT("/UserAudioType");
	wxString userAudioOutDevKey = wxT("/") + name + wxT("/UserAudioOutDev");
	wxString userAudioInDevKey  = wxT("/") + name + wxT("/UserAudioInDev");
	wxString sdrAudioTypeKey    = wxT("/") + name + wxT("/SDRAudioType");
	wxString sdrAudioOutDevKey  = wxT("/") + name + wxT("/SDRAudioOutDev");
	wxString sdrAudioInDevKey   = wxT("/") + name + wxT("/SDRAudioInDev");
	wxString txInEnableKey      = wxT("/") + name + wxT("/TXInEnable");
	wxString txInDevKey         = wxT("/") + name + wxT("/TXInDev");
	wxString txInPinKey         = wxT("/") + name + wxT("/TXInPin");
	wxString keyInEnableKey     = wxT("/") + name + wxT("/KeyInEnable");
	wxString keyInDevKey        = wxT("/") + name + wxT("/KeyInDev");
	wxString keyInPinKey        = wxT("/") + name + wxT("/KeyInPin");
	wxString txOutDevKey        = wxT("/") + name + wxT("/TXOutDev");
	wxString txOutPinKey        = wxT("/") + name + wxT("/TXOutPin");
	wxString tuningKey          = wxT("/") + name + wxT("/TuningHW");
	wxString extNameKey         = wxT("/") + name + wxT("/ExternalName");
	wxString extAddrKey         = wxT("/") + name + wxT("/ExternalAddrs");

	wxString test;
	if (config->Read(fileNameKey, &test)) {
		int ret = ::wxMessageBox(_("An SDR with the same name already exists. Overwrite?"), _("Overwrite confirmation"), wxYES_NO | wxICON_QUESTION);
		if (ret == wxNO)
			return;
	}

	bool ret = config->Write(fileNameKey, m_filename);
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - FileName"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	ret = config->Write(userAudioTypeKey, m_userAudioType);
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - UserAudioType"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	ret = config->Write(userAudioOutDevKey, m_userAudioOutDev);
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - UserAudioOutDev"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	ret = config->Write(userAudioInDevKey, m_userAudioInDev);
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - UserAudioInDev"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	if (featureList[m_sdrType].sdrAudioButton) {
		ret = config->Write(sdrAudioTypeKey, m_sdrAudioType);
		if (!ret) {
			::wxMessageBox(_("Unable to write configuration data - SDRAudioType"), _("GUISetup Error"), wxICON_ERROR);
			return;
		}

		ret = config->Write(sdrAudioOutDevKey, m_sdrAudioOutDev);
		if (!ret) {
			::wxMessageBox(_("Unable to write configuration data - SDRAudioOutDev"), _("GUISetup Error"), wxICON_ERROR);
			return;
		}

		ret = config->Write(sdrAudioInDevKey, m_sdrAudioInDev);
		if (!ret) {
			::wxMessageBox(_("Unable to write configuration data - SDRAudioInDev"), _("GUISetup Error"), wxICON_ERROR);
			return;
		}
	}

	ret = config->Write(tuningKey, long(m_tuningHW));
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - TuningHW"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	ret = config->Write(extNameKey, m_externalName);
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - ExternalName"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	ret = config->Write(extAddrKey, long(m_externalAddr));
	if (!ret) {
		::wxMessageBox(_("Unable to write configuration data - ExternalAddrs"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

#if defined(__WXMSW__)
	wxString instDirKey = wxT("/InstPath");

	wxString dir;
	bool found = config->Read(instDirKey, &dir);

	if (!found) {
		::wxMessageBox(_("Cannot find the registry key for the\ninstallation directory. Cannot create\nthe start menu entry."), _("GUISetup Error"), wxICON_ERROR);
	} else {
		bool create = m_deskTop->GetValue();
		if (create)
			writeDeskTop(name, dir);
	}
#elif defined(__WXGTK__)
	if (m_deskTop != NULL) {
		bool create = m_deskTop->GetValue();
		if (create) {
			wxString dir;
			getDesktopDir(dir);

			writeDeskTop(name, dir);
		}
	}
#endif

	config->Flush();

	delete config;

	::wxMessageBox(_("UWSDR configuration written."));

	Close(true);
}

void CGUISetupFrame::enumerateConfigs()
{
	wxConfig* config = new wxConfig(wxT("UWSDR"));

	wxString name;
	long n;
	bool ret = config->GetFirstGroup(name, n);

	while (ret) {
		m_name->Append(name);

		ret = config->GetNextGroup(name, n);
	}

	delete config;
}

void CGUISetupFrame::readConfig(const wxString& name)
{
	// Clear everything
	m_sdrAudio->Disable();

	m_filenameText->Clear();

	wxConfig* config = new wxConfig(wxT("UWSDR"));

	wxString fileNameKey        = wxT("/") + name + wxT("/FileName");
	wxString userAudioTypeKey   = wxT("/") + name + wxT("/UserAudioType");
	wxString userAudioInDevKey  = wxT("/") + name + wxT("/UserAudioInDev");
	wxString userAudioOutDevKey = wxT("/") + name + wxT("/UserAudioOutDev");
	wxString sdrAudioTypeKey    = wxT("/") + name + wxT("/SDRAudioType");
	wxString sdrAudioInDevKey   = wxT("/") + name + wxT("/SDRAudioInDev");
	wxString sdrAudioOutDevKey  = wxT("/") + name + wxT("/SDRAudioOutDev");
	wxString txInEnableKey      = wxT("/") + name + wxT("/TXInEnable");
	wxString txInDevKey         = wxT("/") + name + wxT("/TXInDev");
	wxString txInPinKey         = wxT("/") + name + wxT("/TXInPin");
	wxString keyInEnableKey     = wxT("/") + name + wxT("/KeyInEnable");
	wxString keyInDevKey        = wxT("/") + name + wxT("/KeyInDev");
	wxString keyInPinKey        = wxT("/") + name + wxT("/KeyInPin");
	wxString txOutDevKey        = wxT("/") + name + wxT("/TXOutDev");
	wxString txOutPinKey        = wxT("/") + name + wxT("/TXOutPin");
	wxString tuningKey          = wxT("/") + name + wxT("/TuningHW");
	wxString extNameKey         = wxT("/") + name + wxT("/ExternalName");
	wxString extAddrKey         = wxT("/") + name + wxT("/ExternalAddrs");

	bool ret = config->Read(fileNameKey, &m_filename);
	if (!ret)
		return;

	wxFileName filePath(m_filename);
	m_filenameText->SetValue(filePath.GetFullName());

	CSDRDescrFile file(m_filename);
	if (!file.isValid()) {
		::wxMessageBox(_("Cannot open the SDR File"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	config->Read(userAudioTypeKey,   (int*)&m_userAudioType);
	config->Read(userAudioInDevKey,  &m_userAudioInDev);
	config->Read(userAudioOutDevKey, &m_userAudioOutDev);

	config->Read(sdrAudioTypeKey,   (int*)&m_sdrAudioType);
	config->Read(sdrAudioInDevKey,  &m_sdrAudioInDev);
	config->Read(sdrAudioOutDevKey, &m_sdrAudioOutDev);

	config->Read(tuningKey,      (int*)&m_tuningHW);

	config->Read(extNameKey,     &m_externalName);

	config->Read(extAddrKey,     (int*)&m_externalAddr);

	delete config;

	m_sdrType = file.getType();

	if (featureList[m_sdrType].sdrAudioButton)
		m_sdrAudio->Enable();
}

void CGUISetupFrame::onUserAudio(wxCommandEvent& WXUNUSED(event))
{
	CSoundCardDialog dialog(this, _("User Audio Setup"), m_userAudioType, m_userAudioInDev, m_userAudioOutDev, 1U, 2U);

	int ret = dialog.ShowModal();
	if (ret == wxID_OK) {
		m_userAudioType   = dialog.getType();
		m_userAudioInDev  = dialog.getInDev();
		m_userAudioOutDev = dialog.getOutDev();
	}
}

void CGUISetupFrame::onSDRAudio(wxCommandEvent& WXUNUSED(event))
{
	CSoundCardDialog dialog(this, _("SDR Audio Setup"), m_sdrAudioType, m_sdrAudioInDev, m_sdrAudioOutDev, 2U, 2U);

	int ret = dialog.ShowModal();
	if (ret == wxID_OK) {
		m_sdrAudioType   = dialog.getType();
		m_sdrAudioInDev  = dialog.getInDev();
		m_sdrAudioOutDev = dialog.getOutDev();
	}
}

void CGUISetupFrame::onTuning(wxCommandEvent& WXUNUSED(event))
{
	CTuningDialog dialog(this, _("Tuning Hardware Setup"), m_tuningHW);

	int ret = dialog.ShowModal();
	if (ret != wxID_OK)
		return;

	m_tuningHW = dialog.getTuningHW();
}

void CGUISetupFrame::onExternal(wxCommandEvent& WXUNUSED(event))
{
	CExternalDialog dialog(this, _("External Program Setup"), m_externalName, m_externalAddr);

	int ret = dialog.ShowModal();
	if (ret != wxID_OK)
		return;

	m_externalName = dialog.getName();
	m_externalAddr = dialog.getAddr();
}

#if defined(__WXMSW__)

void CGUISetupFrame::writeDeskTop(const wxString& name, const wxString& dir)
{
	TCHAR folder[MAX_PATH];
	BOOL res = ::SHGetSpecialFolderPath(NULL, folder, CSIDL_DESKTOP, FALSE);

	if (!res) {
		::wxMessageBox(_("Cannot get the Desktop folder from Windows"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	wxString linkName = name + wxT(".lnk");
	wxString linkPath = wxString(folder) + wxT("\\") + linkName;
	wxString exePath  = dir + wxT("\\UWSDR.exe");
	wxString args     = name;

	HRESULT hRes = ::CoInitialize(NULL);
	if (!SUCCEEDED(hRes)) {
		::wxMessageBox(_("Cannot initialise the COM interface"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	IShellLink* pShellLink;
	hRes = ::CoCreateInstance(CLSID_ShellLink, NULL,
								CLSCTX_INPROC_SERVER,
								IID_IShellLink,
								(void**)&pShellLink);

	if (!SUCCEEDED(hRes)) {
		::CoUninitialize();
		::wxMessageBox(_("Cannot create a COM interface"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	pShellLink->SetPath(exePath.c_str());
	pShellLink->SetArguments(args.c_str());
	pShellLink->SetWorkingDirectory(dir.c_str());

    TCHAR wszLinkfile[MAX_PATH];
	::MultiByteToWideChar(CP_ACP, 0, linkPath.mb_str(), -1, wszLinkfile, MAX_PATH);

    IPersistFile* pPersistFile;
	hRes = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);

	if (!SUCCEEDED(hRes)) {
		pShellLink->Release();
		::CoUninitialize();
		::wxMessageBox(_("Cannot query the COM interface"), _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	hRes = pPersistFile->Save(LPCOLESTR(wszLinkfile), TRUE);
	if (!SUCCEEDED(hRes))
		::wxMessageBox(_("Cannot save the shortcut file"), _("GUISetup Error"), wxICON_ERROR);

	pPersistFile->Release();
	pShellLink->Release();

	::CoUninitialize();
}

#elif defined(__WXGTK__)

void CGUISetupFrame::writeDeskTop(const wxString& name, const wxString& dir)
{
	wxString fileName;
	fileName.Printf(wxT("%s/%s"), wxT(DATA_DIR), SKELETON_FILE);

	// Open the .desktop template file
	wxTextFile inFile;
	bool ret = inFile.Open(fileName);
	if (!ret) {
		::wxMessageBox(_("Cannot read file: ") + fileName, _("GUISetup Error"), wxICON_ERROR);
		return;
	}

	fileName.Printf(wxT("%s/%s.desktop"), dir.c_str(), name.c_str());

	// Remove the old file so that Create will work below
	if (::wxFileExists(fileName))
		::wxRemoveFile(fileName);

	wxTextFile outFile;
	ret = outFile.Create(fileName);
	if (!ret) {
		::wxMessageBox(_("Cannot create file: ") + fileName, _("GUISetup Error"), wxICON_ERROR);
		inFile.Close();
		return;
	}

	wxString line = inFile.GetFirstLine();

	while (!inFile.Eof()) {
		line.Replace(NAME_TOKEN, name, true);

		outFile.AddLine(line);

		line = inFile.GetNextLine();
	}

	outFile.Write();
	outFile.Close();

	inFile.Close();
}

bool CGUISetupFrame::getDesktopDir(wxString& dir) const
{
	bool ret = ::wxGetEnv(HOME_ENV, &dir);

	if (!ret)
		return false;

	dir.Append(DESKTOP_DIR);

	return wxDir::Exists(dir);
}

#endif
