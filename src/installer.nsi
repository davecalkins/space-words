;*******************************************************************************
; This script is compiled with the Nullsoft Scriptable Install System (NSIS)
; to produce a windows installer.
;*******************************************************************************

;*******************************************************************************
; ATTRIBUTES
;*******************************************************************************

Name                 "Space Words"
OutFile              "SpaceWords-installer.exe"
InstallDir           "$PROGRAMFILES\SpaceWords"
BrandingText         " "

CRCCheck             force
SetCompress          force
SetCompressor        lzma

ShowInstDetails      show
ShowUnInstDetails    show

LicenseData          readme.rtf
LicenseText          Overview Continue
LicenseBkColor       /gray

InstallColors        /windows
XPStyle              on

Page                 license
Page                 directory
Page                 instfiles

UninstPage           uninstConfirm
UninstPage           instfiles

;*******************************************************************************
; INSTALLER
;*******************************************************************************

Section ""

   ; install files
   SetOutPath        $INSTDIR
   File              "SpaceWords.exe"
   File              "alleg42.dll"
   File              "alogg.dll"
   File              /r "data"
   File              readme.rtf

   ; create shortcuts
   SetOutPath        $INSTDIR
   CreateShortcut    "$DESKTOP\Space Words.lnk" "$INSTDIR\SpaceWords.exe"
   CreateDirectory   "$STARTMENU\Programs\Space Words"
   CreateShortcut    "$STARTMENU\Programs\Space Words\Space Words.lnk" "$INSTDIR\SpaceWords.exe"
   CreateShortcut    "$STARTMENU\Programs\Space Words\README.lnk" "$INSTDIR\readme.rtf"
   
   ; create the un-installer   
   WriteUninstaller  Uninst.exe
   
   ; create registry entries to allow uninstall via Add/Remove programs control panel
   WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpaceWords" "DisplayName" "Space Words"
   WriteRegStr       HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpaceWords" "UninstallString" "$INSTDIR\Uninst.exe"

SectionEnd

;*******************************************************************************
; UNINSTALLER
;*******************************************************************************

Section "Uninstall"
   RMDir             /r $INSTDIR
   RMDir             /r "$STARTMENU\Programs\Space Words"
   Delete            "$DESKTOP\Space Words.lnk"
   DeleteRegKey      HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpaceWords"
SectionEnd
