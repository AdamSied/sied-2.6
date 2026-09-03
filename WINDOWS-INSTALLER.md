# Creating the SIED Windows installer

## One-command release

Merge the factory one-shot and texture packs into `FactoryLibrary` first. Open the Visual Studio
Developer PowerShell in the `sied-vst` folder and run:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
.\scripts\build-windows.ps1 -Installer -InstallInnoSetup
```

This builds and validates the 64-bit VST3, verifies all 339 factory files, and produces:

```text
dist\SIED-2.6.0-Windows-Setup.exe
```

`-InstallInnoSetup` uses `winget` to install Inno Setup 6 only when its compiler is not already
available. Omit that switch after the first successful setup.

## What a friend's computer receives

- `C:\Program Files\Common Files\VST3\SIED.vst3`
- `C:\ProgramData\SIED\Library\Oneshots`
- `C:\ProgramData\SIED\Library\Textures`
- A SIED entry under Windows **Installed apps**, including an uninstaller

The friend does not need Visual Studio, CMake, Git, JUCE, or Inno Setup. After installation,
rescan VST3 plugins in FL Studio or another 64-bit VST3 host.

## Rebuilding only the installer

If the VST3 has already compiled successfully:

```powershell
.\scripts\build-installer-windows.ps1 -InstallInnoSetup
```

The script prints the installer's SHA-256 checksum after creation. This value can be shared with
testers so they can verify that the file arrived unchanged.

## Plugin-only installer in GitHub Actions

Upload the complete extracted source project, open **Actions**, select **Build Windows installer**,
and choose **Run workflow**. The cloud build returns
`SIED-2.6.0-Windows-Plugin-Setup.exe`. This smaller installer intentionally omits the large sound
bank; copy or install the factory packs separately. The local command for the same package is:

```powershell
.\scripts\build-windows.ps1 -Installer -PluginOnlyInstaller
```

## Distribution checks

This installer is not code-signed, so Windows may show an **Unknown publisher** or SmartScreen
warning. A public release should use a trusted Windows code-signing certificate. Also verify the
current JUCE terms and the redistribution terms of every included factory sound before sharing
the installer outside a private test group.
