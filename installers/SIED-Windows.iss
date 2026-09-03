#define SiedVersion "2.6.0"
#define SiedPublisher "Sied Audio"
#define SiedName "SIED"

[Setup]
AppId={{739F12CC-A9A4-4878-9B81-5D597DC73E09}
AppName={#SiedName}
AppVersion={#SiedVersion}
AppVerName={#SiedName} {#SiedVersion}
AppPublisher={#SiedPublisher}
VersionInfoCompany={#SiedPublisher}
VersionInfoDescription=SIED Windows VST3 installer
VersionInfoProductName={#SiedName}
VersionInfoProductVersion={#SiedVersion}
VersionInfoVersion={#SiedVersion}
DefaultDirName={autopf}\Sied Audio\SIED
DefaultGroupName=SIED
DisableProgramGroupPage=yes
OutputDir=..\dist
#ifdef PluginOnly
OutputBaseFilename=SIED-{#SiedVersion}-Windows-Plugin-Setup
#else
OutputBaseFilename=SIED-{#SiedVersion}-Windows-Setup
#endif
Compression=lzma2/max
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName=SIED {#SiedVersion}
CloseApplications=yes
RestartApplications=no
SetupLogging=yes
MinVersion=10.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\build\SIED_artefacts\Release\VST3\SIED.vst3\*"; DestDir: "{commoncf64}\VST3\SIED.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs
#ifndef PluginOnly
Source: "..\FactoryLibrary\Oneshots\*"; DestDir: "{commonappdata}\SIED\Library\Oneshots"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\FactoryLibrary\Textures\*"; DestDir: "{commonappdata}\SIED\Library\Textures"; Flags: ignoreversion recursesubdirs createallsubdirs
#endif
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\Assets\UI\DejaVu-LICENSE.txt"; DestDir: "{app}"; Flags: ignoreversion

[Dirs]
#ifndef PluginOnly
Name: "{commonappdata}\SIED\Library\Oneshots"
Name: "{commonappdata}\SIED\Library\Textures"
#endif

[Icons]
Name: "{group}\SIED Readme"; Filename: "{app}\README.md"
Name: "{group}\Uninstall SIED"; Filename: "{uninstallexe}"

[Messages]
FinishedLabel=Setup has finished installing SIED on your computer.%n%nOpen your DAW, rescan VST3 plugins, and load SIED.
