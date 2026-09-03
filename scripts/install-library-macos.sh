#!/usr/bin/env bash
set -euo pipefail

project_root="$(cd "$(dirname "$0")/.." && pwd)"
library_source="$project_root/FactoryLibrary"
library_destination="$HOME/Documents/SIED/Library"

if [[ ! -d "$library_source" ]]; then
  echo "FactoryLibrary was not found beside the SIED source folder." >&2
  exit 1
fi

mkdir -p "$library_destination/Oneshots" "$library_destination/Textures"
cp -R "$library_source/Oneshots/." "$library_destination/Oneshots/"
cp -R "$library_source/Textures/." "$library_destination/Textures/"
echo "SIED factory library installed to $library_destination"
