; Inno Setup script. Build: iscc /DMyVersion=1.2.3 /DSourceVst3=path\to\"Furchild.vst3" /DManualPdf=path\to\manual.pdf Furchild.iss
#ifndef MyVersion
  #define MyVersion "0.0.0"
#endif
#ifndef SourceVst3
  #define SourceVst3 "..\..\build\Furchild_artefacts\Release\VST3\Furchild.vst3"
#endif
[Setup]
AppId={{5B8E1C3A-7D42-4F96-B1A0-C2E4D9F37A15}
AppName=Furchild
AppVersion={#MyVersion}
AppPublisher=Studio 87
AppPublisherURL=https://github.com/SpeakingColors/furchild
DefaultDirName={autopf}\Studio 87\Furchild
DisableProgramGroupPage=yes
ArchitecturesInstallIn64BitMode=x64compatible
OutputBaseFilename=Furchild-Setup
Compression=lzma2
SolidCompression=yes
LicenseFile=..\..\LICENSE
UninstallDisplayName=Furchild
CloseApplications=yes
[Messages]
SelectDirDesc=Where should the manual and licence go?
SelectDirLabel3=The plugin itself is installed to the common VST3 folder (C:\Program Files\Common Files\VST3). This folder only holds the manual, the licence and the uninstaller.
[Files]
Source: "{#SourceVst3}\*"; DestDir: "{commoncf64}\VST3\Furchild.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion
#ifdef ManualPdf
Source: "{#ManualPdf}"; DestDir: "{app}"; DestName: "Furchild-manual.pdf"; Flags: ignoreversion
#endif
