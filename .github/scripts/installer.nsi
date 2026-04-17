; ─────────────────────────────────────────────────────────────────────────────
; CrushClone  —  Windows NSIS Installer
;
; Invoked by the GitHub Actions workflow as:
;   makensis.exe /DVERSION=1.2.0 /DPLUGIN_NAME=CrushClone /DBUILD_TYPE=Release /DREPO_ROOT=C:\... installer.nsi
;
; Installs:
;   VST3  →  C:\Program Files\Common Files\VST3\CrushClone.vst3\
;
; Adds a proper Add/Remove Programs entry so users can uninstall cleanly.
; ─────────────────────────────────────────────────────────────────────────────

Unicode True

; ── Compile-time defines (all overridable from the command line) ──────────────
!ifndef VERSION
  !define VERSION    "dev"
!endif
!ifndef PLUGIN_NAME
  !define PLUGIN_NAME "CrushClone"
!endif
!ifndef BUILD_TYPE
  !define BUILD_TYPE  "Release"
!endif
!ifndef REPO_ROOT
  !define REPO_ROOT   "."
!endif

!define PUBLISHER    "Doxie Dev Ops"
!define WEBSITE_URL  "https://github.com/doxiedevops/CrushClone-releases"
!define REGKEY       "Software\${PUBLISHER}\${PLUGIN_NAME}"
!define UNINSTKEY    "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PLUGIN_NAME}"

; Path to the VST3 bundle produced by CMake
; (a folder: CrushClone.vst3\Contents\x86_64-win\CrushClone.dll)
!define VST3_BUNDLE "${REPO_ROOT}\build\${PLUGIN_NAME}_artefacts\${BUILD_TYPE}\VST3\${PLUGIN_NAME}.vst3"

; ── Output file (absolute so it always lands in repo-root\dist\) ─────────────
OutFile "${REPO_ROOT}\dist\${PLUGIN_NAME}-v${VERSION}-Windows.exe"

; ── General ───────────────────────────────────────────────────────────────────
Name            "${PLUGIN_NAME} v${VERSION}"
InstallDir      "$COMMONFILES64\VST3"
InstallDirRegKey HKLM "${REGKEY}" "InstallDir"
RequestExecutionLevel admin
SetCompressor   /SOLID lzma
CRCCheck        on
ShowInstDetails show

; ── MUI2 Pages ────────────────────────────────────────────────────────────────
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

!define MUI_ABORTWARNING
!define MUI_ICON   "${NSISDIR}\Contrib\Graphics\Icons\modern-install.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"

!define MUI_WELCOMEPAGE_TITLE "${PLUGIN_NAME} v${VERSION} Installer"
!define MUI_WELCOMEPAGE_TEXT  \
  "This wizard will install ${PLUGIN_NAME} v${VERSION} as a VST3 plug-in.$\r$\n\
   $\r$\n\
   The plug-in will be installed to:$\r$\n\
   C:\Program Files\Common Files\VST3$\r$\n\
   $\r$\n\
   Click Next to continue."

!define MUI_FINISHPAGE_TITLE  "${PLUGIN_NAME} v${VERSION} Installed"
!define MUI_FINISHPAGE_TEXT   \
  "${PLUGIN_NAME} has been installed successfully.$\r$\n\
   $\r$\n\
   Restart your DAW to pick up the new plug-in.$\r$\n\
   $\r$\n\
   Visit ${WEBSITE_URL} for updates and release notes."

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; ─────────────────────────────────────────────────────────────────────────────
; Installer Section — VST3
; ─────────────────────────────────────────────────────────────────────────────
Section "${PLUGIN_NAME} VST3" SecVST3

  ; Guard: ensure we're running on 64-bit Windows
  ${IfNot} ${RunningX64}
    MessageBox MB_ICONSTOP "This plug-in requires a 64-bit version of Windows."
    Abort
  ${EndIf}

  SetOutPath "$INSTDIR"

  ; Copy the entire .vst3 folder (bundle) recursively
  File /r "${VST3_BUNDLE}"

  ; Write install location to registry (used by the installer next time round)
  WriteRegStr   HKLM "${REGKEY}" "InstallDir"  "$INSTDIR"
  WriteRegStr   HKLM "${REGKEY}" "Version"     "${VERSION}"

  ; Add/Remove Programs entry
  WriteRegStr   HKLM "${UNINSTKEY}" "DisplayName"          "${PLUGIN_NAME}"
  WriteRegStr   HKLM "${UNINSTKEY}" "DisplayVersion"       "${VERSION}"
  WriteRegStr   HKLM "${UNINSTKEY}" "Publisher"            "${PUBLISHER}"
  WriteRegStr   HKLM "${UNINSTKEY}" "URLInfoAbout"         "${WEBSITE_URL}"
  WriteRegStr   HKLM "${UNINSTKEY}" "UninstallString"      '"$INSTDIR\Uninstall_${PLUGIN_NAME}.exe"'
  WriteRegStr   HKLM "${UNINSTKEY}" "QuietUninstallString" '"$INSTDIR\Uninstall_${PLUGIN_NAME}.exe" /S'
  WriteRegDWORD HKLM "${UNINSTKEY}" "NoModify"             1
  WriteRegDWORD HKLM "${UNINSTKEY}" "NoRepair"             1

  WriteUninstaller "$INSTDIR\Uninstall_${PLUGIN_NAME}.exe"

SectionEnd

; ─────────────────────────────────────────────────────────────────────────────
; Uninstaller
; ─────────────────────────────────────────────────────────────────────────────
Section "Uninstall"

  ; Remove the VST3 bundle folder
  RMDir /r "$INSTDIR\${PLUGIN_NAME}.vst3"

  ; Remove the uninstaller itself
  Delete "$INSTDIR\Uninstall_${PLUGIN_NAME}.exe"

  ; Remove registry entries
  DeleteRegKey HKLM "${REGKEY}"
  DeleteRegKey HKLM "${UNINSTKEY}"

  ; Remove install dir only if empty (don't nuke Common Files\VST3 if other plugins live there)
  RMDir "$INSTDIR"

SectionEnd
