;NSIS Modern User Interface version 1.70
;UWSDR install script
;Written by Jonathan Naylor

  SetCompressor lzma

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  Name "UWSDR 0.9.1"
  OutFile "UWSDR-0.9.1.exe"

  ;Folder selection page
  InstallDir "$PROGRAMFILES\UWSDR"
  
  ;Get install folder from registry if available
  InstallDirRegKey HKCU "Software\UWSDR" "InstPath"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNI3NG

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Copying.txt"
  !insertmacro MUI_PAGE_DIRECTORY

  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "UWSDR Program Files" SecProgram

  CreateDirectory "$INSTDIR\SDR Files"

  SetOutPath "$INSTDIR"
  
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\UWSDR.exe"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\GUISetup.exe"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\SDRHelp.exe"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\libfftw3f-3.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\libusb-1.0.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\libusb0.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\portaudio_x86.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\wxbase28u_vc_custom.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\wxbase28u_net_vc_custom.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\wxmsw28u_core_vc_custom.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Release\wxmsw28u_html_vc_custom.dll"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Help\GUISetup\GUISetup.zip"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Help\UWSDR\UWSDR.zip"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Copying.txt"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Changes.txt"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Install.txt"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\Jobs.txt"

  SetOutPath "$INSTDIR\SDR Files"

  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\Readme.txt"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\CCW-SDR-4+.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\CCW-Sentry.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\Demo.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\HackRF.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\HackRF-HF.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\HackRF-HF-RX.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\HackRF-RX.sdr"
  File "C:\Users\Jonathan\Documents\Visual Studio 2008\Projects\uwsdr\Software\SDRFiles\Skeleton.sdr"

  ;Create start menu entry
  CreateDirectory "$SMPROGRAMS\UWSDR"
  CreateShortCut "$SMPROGRAMS\UWSDR\UWSDR.lnk"     "$INSTDIR\UWSDR.exe"
  CreateShortCut "$SMPROGRAMS\UWSDR\GUISetup.lnk"  "$INSTDIR\GUISetup.exe"
  CreateShortCut "$SMPROGRAMS\UWSDR\SDRHelp.lnk"   "$INSTDIR\SDRHelp.exe"
; CreateShortCut "$SMPROGRAMS\UWSDR\SDRSetup.lnk"  "$INSTDIR\SDRSetup.exe"
  CreateShortCut "$SMPROGRAMS\UWSDR\Licence.lnk"   "$INSTDIR\Copying.txt"
  CreateShortCut "$SMPROGRAMS\UWSDR\Changes.lnk"   "$INSTDIR\Changes.txt"
  CreateShortCut "$SMPROGRAMS\UWSDR\Install.lnk"   "$INSTDIR\Install.txt"
  CreateShortCut "$SMPROGRAMS\UWSDR\TODO.lnk"      "$INSTDIR\Jobs.txt"
  CreateShortCut "$SMPROGRAMS\UWSDR\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  ;Store install folder
  WriteRegStr HKCU "Software\UWSDR" "InstPath" $INSTDIR

  ;Store Windows uninstall keys
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UWSDR" "DisplayName"      "UWSDR"
  WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UWSDR" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UWSDR" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UWSDR" "NoRepair" 1

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\SDR Files\*.*"
  RMDir  "$INSTDIR\SDR Files"

  Delete "$INSTDIR\*.*"
  RMDir  "$INSTDIR"

  Delete "$SMPROGRAMS\UWSDR\*.*"
  RMDir  "$SMPROGRAMS\UWSDR"

  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\UWSDR"

  DeleteRegKey HKCU "Software\UWSDR"

SectionEnd
