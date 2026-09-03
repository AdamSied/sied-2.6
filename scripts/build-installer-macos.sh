#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
version="2.6.0"
plugin_only=false

if [[ "${1:-}" == "--plugin-only" ]]; then
  plugin_only=true
elif [[ $# -gt 0 ]]; then
  echo "Usage: $0 [--plugin-only]" >&2
  exit 2
fi

for command_name in pkgbuild productbuild codesign ditto; do
  if ! command -v "$command_name" >/dev/null 2>&1; then
    echo "$command_name is required to create the macOS installer." >&2
    exit 1
  fi
done

vst3_source="$project_root/build/SIED_artefacts/Release/VST3/SIED.vst3"
au_source="$project_root/build/SIED_artefacts/Release/AU/SIED.component"
if [[ ! -x "$vst3_source/Contents/MacOS/SIED" || ! -x "$au_source/Contents/MacOS/SIED" ]]; then
  echo "Compiled SIED VST3 and AU bundles were not found. Run build-macos.sh first." >&2
  exit 1
fi

mkdir -p "$project_root/build" "$project_root/dist"
temporary_root="$(mktemp -d "$project_root/build/sied-macos-installer.XXXXXX")"
trap 'rm -rf "$temporary_root"' EXIT
payload_root="$temporary_root/payload"
mkdir -p \
  "$payload_root/Library/Audio/Plug-Ins/VST3" \
  "$payload_root/Library/Audio/Plug-Ins/Components" \
  "$payload_root/Library/Application Support/SIED"

ditto "$vst3_source" "$payload_root/Library/Audio/Plug-Ins/VST3/SIED.vst3"
ditto "$au_source" "$payload_root/Library/Audio/Plug-Ins/Components/SIED.component"
ditto "$project_root/README.md" "$payload_root/Library/Application Support/SIED/README.md"
ditto "$project_root/Assets/UI/DejaVu-LICENSE.txt" \
  "$payload_root/Library/Application Support/SIED/DejaVu-LICENSE.txt"

if [[ "$plugin_only" == false ]]; then
  library_source="$project_root/FactoryLibrary"
  if [[ ! -d "$library_source/Oneshots" || ! -d "$library_source/Textures" ]]; then
    echo "FactoryLibrary/Oneshots and FactoryLibrary/Textures are required for the full installer." >&2
    exit 1
  fi
  oneshot_count="$(find "$library_source/Oneshots" -type f 2>/dev/null | wc -l | tr -d ' ')"
  texture_count="$(find "$library_source/Textures" -type f 2>/dev/null | wc -l | tr -d ' ')"
  if [[ "$oneshot_count" != "226" || "$texture_count" != "113" ]]; then
    echo "Factory library is incomplete. Expected 226 one-shots and 113 textures; found $oneshot_count and $texture_count." >&2
    exit 1
  fi
  mkdir -p "$payload_root/Library/Application Support/SIED/Library"
  ditto "$library_source/Oneshots" \
    "$payload_root/Library/Application Support/SIED/Library/Oneshots"
  ditto "$library_source/Textures" \
    "$payload_root/Library/Application Support/SIED/Library/Textures"
fi

application_identity="${SIED_MAC_APPLICATION_IDENTITY:--}"
if [[ "$application_identity" == "-" ]]; then
  codesign --force --deep --sign - "$payload_root/Library/Audio/Plug-Ins/VST3/SIED.vst3"
  codesign --force --deep --sign - "$payload_root/Library/Audio/Plug-Ins/Components/SIED.component"
else
  codesign --force --deep --options runtime --timestamp --sign "$application_identity" \
    "$payload_root/Library/Audio/Plug-Ins/VST3/SIED.vst3"
  codesign --force --deep --options runtime --timestamp --sign "$application_identity" \
    "$payload_root/Library/Audio/Plug-Ins/Components/SIED.component"
fi

component_package="$temporary_root/SIED-component.pkg"
pkgbuild \
  --root "$payload_root" \
  --identifier "audio.sied.installer" \
  --version "$version" \
  --install-location "/" \
  "$component_package"

if [[ "$plugin_only" == true ]]; then
  output_package="$project_root/dist/SIED-${version}-macOS-Plugin.pkg"
else
  output_package="$project_root/dist/SIED-${version}-macOS-Setup.pkg"
fi

unsigned_package="$temporary_root/SIED-unsigned.pkg"
productbuild --package "$component_package" "$unsigned_package"

installer_identity="${SIED_MAC_INSTALLER_IDENTITY:-}"
if [[ -n "$installer_identity" ]]; then
  productsign --sign "$installer_identity" "$unsigned_package" "$output_package"
else
  ditto "$unsigned_package" "$output_package"
fi

if [[ ! -f "$output_package" ]]; then
  echo "The macOS installer was not created." >&2
  exit 1
fi

echo "macOS installer created: $output_package"
shasum -a 256 "$output_package"
