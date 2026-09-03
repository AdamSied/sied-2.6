param(
    [switch]$Installer,
    [switch]$InstallInnoSetup,
    [switch]$PluginOnlyInstaller
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest
$ProjectRoot = Split-Path -Parent $PSScriptRoot

function Invoke-NativeCommand {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name,

        [Parameter(Mandatory = $true)]
        [scriptblock]$Command
    )

    & $Command
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE. The release was not created."
    }
}

$CMakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
if ($null -eq $CMakeCommand) {
    throw "CMake was not found. Open the Visual Studio Developer PowerShell and try again."
}

$CapabilitiesJson = & cmake -E capabilities
if ($LASTEXITCODE -ne 0) {
    throw "CMake could not report its available generators."
}
$Capabilities = $CapabilitiesJson | ConvertFrom-Json
$GeneratorNames = $Capabilities.generators | ForEach-Object { $_.name }

if ($GeneratorNames -contains "Visual Studio 18 2026") {
    $Generator = "Visual Studio 18 2026"
}
elseif ($GeneratorNames -contains "Visual Studio 17 2022") {
    $Generator = "Visual Studio 17 2022"
}
else {
    throw "SIED requires Visual Studio 2026 or 2022 with Desktop development with C++."
}

Write-Host "Using $Generator"
Invoke-NativeCommand -Name "CMake configuration" -Command {
    & cmake -S $ProjectRoot -B "$ProjectRoot/build" -G $Generator -A x64
}
Invoke-NativeCommand -Name "SIED compilation" -Command {
    & cmake --build "$ProjectRoot/build" --config Release --target SIED_VST3 SIED_Standalone
}

$Vst3Bundle = Join-Path $ProjectRoot "build\SIED_artefacts\Release\VST3\SIED.vst3"
$Vst3Binary = Get-ChildItem -Path $Vst3Bundle -Recurse -File -Filter "SIED.vst3" -ErrorAction SilentlyContinue |
    Select-Object -First 1
if (-not (Test-Path $Vst3Bundle -PathType Container) -or $null -eq $Vst3Binary) {
    throw "Compilation ended without producing a valid SIED.vst3 bundle at $Vst3Bundle."
}

$LibrarySource = Join-Path $ProjectRoot "FactoryLibrary"
$Documents = [Environment]::GetFolderPath("MyDocuments")
$LibraryDestination = Join-Path $Documents "SIED\Library"
if (Test-Path $LibrarySource) {
    foreach ($Section in @("Oneshots", "Textures")) {
        $SectionSource = Join-Path $LibrarySource $Section
        $SectionDestination = Join-Path $LibraryDestination $Section
        if (Test-Path $SectionSource) {
            New-Item -ItemType Directory -Force -Path $SectionDestination | Out-Null
            Copy-Item -Path (Join-Path $SectionSource "*") -Destination $SectionDestination -Recurse -Force
        }
    }
    Write-Host "Factory library installed to $LibraryDestination"
}

Write-Host "Build succeeded: $Vst3Bundle"

if ($Installer) {
    $InstallerArguments = @{}
    if ($InstallInnoSetup) {
        $InstallerArguments.InstallInnoSetup = $true
    }
    if ($PluginOnlyInstaller) {
        $InstallerArguments.PluginOnly = $true
    }

    & (Join-Path $PSScriptRoot "build-installer-windows.ps1") @InstallerArguments
    if (-not $?) {
        throw "The plugin built successfully, but the Windows installer build failed."
    }
}
