; Inno Setup Script — Punic Wars: Castra
; Компілювати: iscc punic_wars_setup.iss
; Або відкрити в Inno Setup IDE та натиснути Build

#define AppName "Punic Wars: Castra"
#define AppVersion "1.0"
#define AppPublisher "Punic Wars Dev Team"
#define AppExeName "punic_wars.exe"
#define SourceDir ".."

[Setup]
AppId={{A1B2C3D4-E5F6-7890-ABCD-EF1234567890}
AppName={#AppName}
AppVersion={#AppVersion}
AppPublisher={#AppPublisher}
DefaultDirName={autopf}\PunicWarsCastra
DefaultGroupName={#AppName}
AllowNoIcons=yes
OutputDir=output
OutputBaseFilename=PunicWarsCastra_Setup_v{#AppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
ArchitecturesInstallIn64BitMode=x64

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional icons:"; Flags: unchecked

[Files]
; Головний виконуваний файл
Source: "{#SourceDir}\punic_wars.exe"; DestDir: "{app}"; Flags: ignoreversion

; Assets — звуки
Source: "{#SourceDir}\assets\sounds\*"; DestDir: "{app}\assets\sounds"; Flags: ignoreversion recursesubdirs createallsubdirs

; Assets — спрайти
Source: "{#SourceDir}\assets\sprites\*"; DestDir: "{app}\assets\sprites"; Flags: ignoreversion recursesubdirs createallsubdirs

; Assets — тайли
Source: "{#SourceDir}\assets\tiles\*"; DestDir: "{app}\assets\tiles"; Flags: ignoreversion recursesubdirs createallsubdirs

; Assets — фони та інше
Source: "{#SourceDir}\assets\Background.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourceDir}\assets\Background2.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourceDir}\assets\Logo.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourceDir}\assets\Pause_background.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourceDir}\assets\Settings_background.png"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#SourceDir}\assets\isometric_tileset.png"; DestDir: "{app}\assets"; Flags: ignoreversion

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"
Name: "{group}\Uninstall {#AppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Launch {#AppName}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: filesandordirs; Name: "{app}"
