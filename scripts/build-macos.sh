#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
build_installer=false
plugin_only_installer=false

for argument in "$@"; do
  case "$argument" in
    --installer)
      build_installer=true
      ;;
    --plugin-only-installer)
      build_installer=true
      plugin_only_installer=true
      ;;
    *)
      echo "Unknown option: $argument" >&2
      echo "Usage: $0 [--installer | --plugin-only-installer]" >&2
      exit 2
      ;;
  esac
done

cmake -S "$project_root" -B "$project_root/build" -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build "$project_root/build" --config Release --target SIED_VST3 SIED_AU SIED_Standalone

library_source="$project_root/FactoryLibrary"
library_destination="$HOME/Documents/SIED/Library"
if [[ -d "$library_source" ]]; then
  mkdir -p "$library_destination/Oneshots" "$library_destination/Textures"
  cp -R "$library_source/Oneshots/." "$library_destination/Oneshots/"
  cp -R "$library_source/Textures/." "$library_destination/Textures/"
  echo "Factory library installed to $library_destination"
fi

vst3_bundle="$project_root/build/SIED_artefacts/Release/VST3/SIED.vst3"
au_bundle="$project_root/build/SIED_artefacts/Release/AU/SIED.component"
if [[ ! -x "$vst3_bundle/Contents/MacOS/SIED" || ! -x "$au_bundle/Contents/MacOS/SIED" ]]; then
  echo "The build ended without producing valid VST3 and AU binaries." >&2
  exit 1
fi

echo "Mac build succeeded: $vst3_bundle"
echo "Mac build succeeded: $au_bundle"

if [[ "$build_installer" == true ]]; then
  if [[ "$plugin_only_installer" == true ]]; then
    "$project_root/scripts/build-installer-macos.sh" --plugin-only
  else
    "$project_root/scripts/build-installer-macos.sh"
  fi
fi
