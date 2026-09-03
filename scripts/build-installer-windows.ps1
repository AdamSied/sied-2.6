param(
    [switch]$InstallInnoSetup,
    [switch]$PluginOnly
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Vst3Bundle = Join-Path $ProjectRoot "build\SIED_artefacts\Release\VST3\SIED.vst3"
$InstallerScript = Join-Path $ProjectRoot "installers\SIED-Windows.iss"
$ExpectedOneshots = 226
$ExpectedTextures = 113

if (-not (Test-Path $Vst3Bundle -PathType Container)) {
    throw "SIED.vst3 was not found at $Vst3Bundle. Run .\scripts\build-windows.ps1 first."
}

$Vst3Binary = Get-ChildItem -Path $Vst3Bundle -Recurse -File -Filter "SIED.vst3" -ErrorAction SilentlyContinue |
    Select-Object -First 1
if ($null -eq $Vst3Binary) {
    throw "The SIED.vst3 folder exists but contains no compiled VST3 binary. Rebuild SIED before creating the installer."
}

if (-not $PluginOnly) {
    $OneshotPath = Join-Path $ProjectRoot "FactoryLibrary\Oneshots"
    $TexturePath = Join-Path $ProjectRoot "FactoryLibrary\Textures"
    $OneshotCount = @(Get-ChildItem -Path $OneshotPath -Recurse -File -ErrorAction SilentlyContinue).Count
    $TextureCount = @(Get-ChildItem -Path $TexturePath -Recurse -File -ErrorAction SilentlyContinue).Count
    if ($OneshotCount -ne $ExpectedOneshots -or $TextureCount -ne $ExpectedTextures) {
        throw "Factory library is incomplete. Expected $ExpectedOneshots one-shots and $ExpectedTextures textures; found $OneshotCount and $TextureCount."
    }
}

function Find-InnoCompiler {
    $FromPath = Get-Command ISCC.exe -ErrorAction SilentlyContinue
    if ($null -ne $FromPath) {
        return $FromPath.Source
    }

    $Candidates = @(
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe",
        "$env:ProgramFiles\Inno Setup 6\ISCC.exe"
    )
    foreach ($Candidate in $Candidates) {
        if (Test-Path $Candidate -PathType Leaf) {
            return $Candidate
        }
    }

    return $null
}

$InnoCompiler = Find-InnoCompiler
if ($null -eq $InnoCompiler -and $InstallInnoSetup) {
    $Winget = Get-Command winget.exe -ErrorAction SilentlyContinue
    if ($null -eq $Winget) {
        throw "Inno Setup is missing and winget is unavailable. Install Inno Setup 6 from jrsoftware.org, then rerun this script."
    }

    Write-Host "Installing Inno Setup 6..."
    & $Winget.Source install --id JRSoftware.InnoSetup --exact --accept-package-agreements --accept-source-agreements
    if ($LASTEXITCODE -ne 0) {
        throw "winget could not install Inno Setup (exit code $LASTEXITCODE)."
    }
    $InnoCompiler = Find-InnoCompiler
}

if ($null -eq $InnoCompiler) {
    throw "Inno Setup 6 is required to create the shareable installer. Rerun with -InstallInnoSetup to install it automatically."
}

New-Item -ItemType Directory -Force -Path (Join-Path $ProjectRoot "dist") | Out-Null
Write-Host "Building the SIED Windows installer..."
$CompilerArguments = @()
if ($PluginOnly) {
    $CompilerArguments += "/DPluginOnly"
}
& $InnoCompiler @CompilerArguments $InstallerScript
if ($LASTEXITCODE -ne 0) {
    throw "The installer compiler failed with exit code $LASTEXITCODE."
}

$InstallerName = if ($PluginOnly) { "SIED-2.6.0-Windows-Plugin-Setup.exe" } else { "SIED-2.6.0-Windows-Setup.exe" }
$Installer = Join-Path $ProjectRoot "dist\$InstallerName"
if (-not (Test-Path $Installer -PathType Leaf)) {
    throw "The installer compiler finished but $Installer was not created."
}

$Hash = (Get-FileHash -Path $Installer -Algorithm SHA256).Hash
Write-Host "Installer created: $Installer"
Write-Host "SHA-256: $Hash"
