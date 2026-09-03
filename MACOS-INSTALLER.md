# Creating the SIED macOS installer

SIED supports a full installer and a smaller plugin-only installer. Both install universal
Apple Silicon and Intel VST3 and Audio Unit bundles.

## No development tools on your Mac

Use the included GitHub workflow:

1. Create a GitHub repository and upload the extracted `sied-vst` project contents.
2. Open the repository's **Actions** tab.
3. Select **Build macOS installer**.
4. Choose **Run workflow**.
5. When it finishes, download the `SIED-2.6.0-macOS-Plugin` artifact.
6. Extract the artifact and double-click `SIED-2.6.0-macOS-Plugin.pkg` on the Mac.

The cloud package contains both:

- `/Library/Audio/Plug-Ins/VST3/SIED.vst3`
- `/Library/Audio/Plug-Ins/Components/SIED.component`

It deliberately omits the large external sound library. Copy the library already installed on
Windows from:

```text
C:\Users\YOUR-NAME\Documents\SIED\Library
```

to the Mac at:

```text
~/Documents/SIED/Library
```

This can be transferred with a USB drive, external SSD, local network, or cloud-storage folder;
the sounds do not need to be downloaded again.

## Complete installer built on a Mac

When the source and all three factory packs are together on a build Mac, run:

```bash
chmod +x scripts/build-macos.sh scripts/build-installer-macos.sh
./scripts/build-macos.sh --installer
```

This creates:

```text
dist/SIED-2.6.0-macOS-Setup.pkg
```

The complete package installs the factory library to:

```text
/Library/Application Support/SIED/Library
```

SIED scans that shared location plus `~/Documents/SIED/Library`. The installed factory copy takes
priority when a name exists in both places, so corrected factory masters stay current without
showing duplicate browser entries.

## Signing and public distribution

The default packages are ad-hoc/unsigned test builds. macOS may require approval under
**System Settings > Privacy & Security**. For a trusted public installer, supply a Developer ID
Application identity in `SIED_MAC_APPLICATION_IDENTITY`, a Developer ID Installer identity in
`SIED_MAC_INSTALLER_IDENTITY`, and notarize the completed package with Apple.
