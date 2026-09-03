$ErrorActionPreference = "Stop"
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$LibrarySource = Join-Path $ProjectRoot "FactoryLibrary"
$Documents = [Environment]::GetFolderPath("MyDocuments")
$LibraryDestination = Join-Path $Documents "SIED\Library"

if (-not (Test-Path $LibrarySource)) {
    throw "FactoryLibrary was not found beside the SIED source folder."
}

foreach ($Section in @("Oneshots", "Textures")) {
    $SectionSource = Join-Path $LibrarySource $Section
    $SectionDestination = Join-Path $LibraryDestination $Section
    New-Item -ItemType Directory -Force -Path $SectionDestination | Out-Null
    Copy-Item -Path (Join-Path $SectionSource "*") -Destination $SectionDestination -Recurse -Force
}

Write-Host "SIED factory library installed to $LibraryDestination"
