# SIED 2.6.0

SIED is a three-layer sample instrument designed first for FL Studio on Windows and macOS.
It combines two playable one-shot layers, one continuously looping texture layer, a focused
sound-design page, and a large factory library.

## What is new in 2.6

- A lighter, warmer, minimal interface with a flatter nostalgic palette and substantially less
  decorative chrome across the header, layer cards, FX rack, waveforms, and Library
- Removed the symbol beside SIED and removed Seed/Variation from both the engine and interface
- Added a dedicated Undo button that restores the complete patch before any one-shot, texture,
  FX, or full-preset randomization, including sound selections and custom file paths
- Rebuilt automatic tuning around several stable analysis frames, strict confidence and pitch
  consistency checks, independent per-layer roots, and automatic cents correction
- Automatic tuning now keeps its cents correction during randomization and falls back to manual
  tuning without altering the sample when analysis is uncertain

## What was new in 2.5

- A quieter, professional interface built around precise hierarchy, neutral surfaces, stronger
  waveform focus, restrained value-reactive light, and functional labels only
- Removed the Aether/engine/status language, ornamental FX numbering, topographic wallpaper, and
  the text beside the main dice action
- A full Library page with live name/category search, source and category filters, one-click local
  favorites, and direct loading into one-shot A, one-shot B, or Texture
- Eighteen persistent randomization locks covering every sound layer, Envelope, Shimmer, Voice,
  and each individual FX module
- Two complete A/B scenes with parameter morphing and midpoint sample switching
- Automatic offline root-note detection for imported one-shots, stored independently for slots A
  and B
- Larger icon-only dice control and a compact performance strip

## What was new in 2.4

- A complete visual redesign with custom knobs, luminous value capsules, animated signal
  feedback, and layered surfaces
- Rebuilt layer-coloured waveforms with a stable, smoothly animated playback marker
- Playback reporting that follows the newest rendered voice and still catches short samples or
  notes that end inside the current audio block
- Musical Release randomization: 10-40% most of the time, with less frequent medium and long tails
- A separate future-feature roadmap in `SIED-FUTURE-IDEAS.md`

## What was new in 2.3

- A cohesive dimensional interface with refined panels, controls, buttons, menus, and waveforms
- Live value-reactive illumination around Shimmer and every FX card
- Slower, linked Amount/Mix response curves that keep low effect settings genuinely subtle
- Balanced full-preset FX randomization without sudden washed-out Reverb or Shimmer
- Louder randomized textures that remain underneath the playable layers but are clearly audible
- User preset saving and loading from the new header browser
- Portable preset recall by both file path and sound name

## What is new in 2.2

- Live playhead markers across all three waveforms while notes are sounding
- Previous/next browser buttons for one-shot A, one-shot B, and the texture layer
- A true dry/wet Mix parameter for every effect, with a cleaner paired-control FX layout
- Rebalanced effect depth so every amount control has an audible, usable sweep
- Stronger high-end Shimmer with a double high-pass feed to keep the larger pad tail clear
- INIT and first launch now use only `key - topograph` in slot A plus `Forest`; slot B is off
- The header preset action is now a focused `RANDOMIZE` dice button
- Random FX now covers Shimmer, every effect mode, delay timing, and all Mix parameters
- Full-preset randomization keeps Shimmer restrained and weights Attack toward 5–15%
- Replaced the external factory collection with 226 corrected one-shots and 113 textures

## What was new in 2.1.1

- Added a one-command Windows release builder that creates a normal shareable `.exe` installer.
- The installer places the VST3 and the factory sounds in their standard folders and adds
  an entry to Windows Installed apps for clean uninstalling.
- Corrected the generated embedded-font identifier that stopped the 2.1 Windows build.
- Windows builds now use the static Visual C++ runtime so a friend's computer does not need a
  separate Visual C++ redistributable merely to scan SIED.
- Build scripts now stop on the first real compiler failure and verify that a non-empty VST3
  bundle exists instead of printing a misleading success message.
- Added a universal Apple Silicon/Intel macOS `.pkg` builder for VST3 and Audio Unit.
- Added a cloud workflow that produces the Mac plugin installer without installing build tools
  on the destination Mac.
- Full Mac installers place factory content in `/Library/Application Support/SIED/Library`, while
  SIED continues to scan the user's `Documents/SIED/Library` for personal and transferred sounds.

## What is new in 2.1

- Per-layer ON/OFF controls with click-free gain smoothing
- Independent transpose (±24 semitones) and fine tune (±100 cents) for both one-shots and texture
- INIT patch button that restores every parameter and reloads the default sound layers
- Host-tempo delay divisions: 1/1, 1/2, 1/4, 1/8, 1/16, dotted, and triplet, plus free milliseconds
- Expanded character modes for Chorus, Drive, Crush, Phaser, Flanger, Tremolo, and Compressor
- Embedded mono display font for cleaner parameter values and matching dark popup menus

## Included from 2.0

- Two independent one-shot slots with their own browser, level, waveform, start, and end
- One quiet texture slot that always loops with a long equal-power crossfade
- Random start position and random-start range for textures
- 226 supplied one-shots and 113 supplied textures, organized as an installable factory library
- Peak normalization when external or factory-library audio is loaded; waveforms are visually normalized
- One-click one-shot, texture, FX, and complete-preset randomizers
- Safe preset randomization that keeps Crush and Drive restrained and textures below the main sound
- 1–32 voice polyphony, Mono mode, and up to two seconds of Glide
- Reworked loop wrap and 128-sample region fades to eliminate held-note clicking
- Clean, Warm, and Resonant low-pass/high-pass modes
- Stereo, Ping-pong, Tape, and Diffuse delay modes
- Room, Hall, Plate, and Cloud reverb modes
- A redesigned minimal interface with consistent buttons, menus, typography, and modern arc knobs
- Shimmer on the front page as SIED's signature control, with an exact +12-semitone layer and a
  high-passed long reverb tail so it stays bright and in tune
- Category-filtered browsing for Keys, Pads, Plucks, Bells, Leads, Bass, Pianos, Synths, and more

## Windows build

If your download came as four ZIP files, extract the source ZIP first. Then extract the
two one-shot packs and the texture pack into that extracted `sied-vst` folder, allowing their shared
`FactoryLibrary` folder to merge. See `FACTORY-LIBRARY-INSTALL.md` for the exact layout.

Install Visual Studio 2026 or 2022 with **Desktop development with C++**, CMake, and Git.
Open the matching Developer PowerShell, move into the extracted `sied-vst` folder, and run:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
./scripts/build-windows.ps1
```

The build script also copies the factory library to:

```text
Documents\SIED\Library
```

Copy `build/SIED_artefacts/Release/VST3/SIED.vst3` to:

```text
C:\Program Files\Common Files\VST3
```

In FL Studio, open **Options > Manage plugins**, enable **Verify plugins**, and choose
**Find installed plugins**.

## Shareable Windows installer

Only the person creating the release needs Visual Studio, CMake, Git, and Inno Setup. Friends
receive one installer and do not need any development tools.

From Visual Studio Developer PowerShell, run this in the `sied-vst` folder:

```powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass -Force
.\scripts\build-windows.ps1 -Installer -InstallInnoSetup
```

The command compiles SIED, installs Inno Setup through Windows Package Manager if it is missing,
checks that all 226 one-shots and 113 textures are present, and creates:

```text
dist\SIED-2.6.0-Windows-Setup.exe
```

Send that `.exe` to a friend. It installs the plugin to
`C:\Program Files\Common Files\VST3\SIED.vst3` and the factory library to
`C:\ProgramData\SIED\Library`. After installation, the friend only needs to rescan VST3 plugins in
the DAW. Because the installer is not code-signed, Windows SmartScreen may show an
**Unknown publisher** warning; public releases should be signed with a trusted code-signing
certificate.

If SIED is already compiled and only the installer needs to be rebuilt, run:

```powershell
.\scripts\build-installer-windows.ps1 -InstallInnoSetup
```

The included `.github/workflows/build-windows-installer.yml` workflow can also create a smaller
plugin-only Windows installer in GitHub Actions. It is useful when the large factory bank is not
stored in the repository; install or transfer the factory packs separately afterward.

Before sharing, confirm that the JUCE license and every included sample license allow your form
of distribution.

If you only need to reinstall the sounds, run:

```powershell
powershell -ExecutionPolicy Bypass -File ./scripts/install-library-windows.ps1
```

## macOS build

Install Xcode, CMake, and Git, then run:

```bash
chmod +x scripts/build-macos.sh
./scripts/build-macos.sh
```

The build installs the sound library to `~/Documents/SIED/Library`. Copy the VST3 to
`~/Library/Audio/Plug-Ins/VST3/` or the AU to `~/Library/Audio/Plug-Ins/Components/`.
FL Studio should use the VST3 version. Unsigned local builds may need approval in
**System Settings > Privacy & Security**.

To build a complete double-clickable installer containing the VST3, AU, and all factory sounds:

```bash
./scripts/build-macos.sh --installer
```

The result is:

```text
dist/SIED-2.6.0-macOS-Setup.pkg
```

### Mac installer without local development tools

The included `.github/workflows/build-macos-installer.yml` workflow builds a universal Mac plugin
in GitHub's Mac cloud and returns `SIED-2.6.0-macOS-Plugin.pkg`. This package contains the VST3 and
AU but not the large external library. Transfer your existing Windows library from:

```text
C:\Users\YOUR-NAME\Documents\SIED\Library
```

to the Mac at:

```text
~/Documents/SIED/Library
```

Then the destination Mac only needs to double-click the `.pkg` and rescan plugins. See
`MACOS-INSTALLER.md` for the cloud steps and signing limitations.

To reinstall only the sounds:

```bash
./scripts/install-library-macos.sh
```

## Using the sample page

Use the category menu to filter the one-shot browsers, or use the arrow buttons to move one sound
at a time. A bright playhead line follows active playback across each waveform. Drag either waveform's coral start
marker or violet end marker to choose a region; double-click to restore the full sample.
The global One-shot/Auto Loop/Loop Region menu and Reverse control apply to both one-shot
slots. Each slot keeps its own start, end, and level.

The texture layer always loops and defaults to a low level. **Start** chooses
its base playhead position. **Random Start** chooses how far that position may move for each
new note. Its long equal-power wrap is designed for click-free held notes. Texture transpose
and fine tune change its playback speed without making it follow the played MIDI note.

Each layer has its own **ON**, **Transpose**, and **Fine** controls. Turning a layer off uses a
short gain ramp so it does not click. **INIT** in the header resets the complete patch.

Shimmer is deliberately on this page. Its pitched component is exactly one octave above the
played note; its separate Mix control blends the complete pitched/tail effect. Chorus remains a
separate modulation control on the FX page.

Every effect has two controls: its named Amount control sets character or intensity, while Mix
sets the dry/wet balance. Their linked tapered response keeps the lower half subtle and reserves
the strongest processing for the upper range. Double-click any knob to return it to its default.

## User presets

Choose **SAVE** in the header, name the preset, and save it. SIED stores the complete patch—including
all parameters, both one-shots, the texture, and custom file paths—in:

```text
Windows: Documents\SIED\Presets
macOS:   ~/Documents/SIED/Presets
```

Saved patches appear in the **USER PRESETS** menu in the header. Factory-library sounds also use a
name fallback, so presets can survive moving the sound library between Windows and macOS.

## Library, favorites, and root detection

Open **LIBRARY** for the full sound browser. Search updates immediately and matches both sound names
and categories. Filter by one-shots or textures, choose the destination slot, and double-click a
result to load it. Click the star at the left of a row to add or remove a favorite; the Favorites
filter shows only those sounds. Favorites are stored locally in `Documents/SIED/favorites.txt`.

When a one-shot is loaded, SIED compares several stable regions before assigning an independent
root and cents correction to that slot. If those regions disagree or confidence is low, SIED leaves
the sample at neutral manual tuning. Analysis never runs on the audio thread and never alters the
texture layer.

## Locks, undo, and scenes

The small lock beside each layer or section protects it from both the main dice and **Random FX**.
Locks are saved with projects and user presets. The Undo button restores the complete patch from
immediately before a randomization and keeps up to 32 earlier randomization states.

Click **A** or **B** to capture the complete current patch into that scene, then drag the control
between them. Continuous parameters interpolate; switches, modes, and sound selections change at
the midpoint. Both scenes are stored inside the project or `.siedpreset` file.

## Randomization behavior

- **Random One-shots** chooses both one-shot sources and lightly varies their regions/levels.
  Each layer keeps the original 0% start half the time instead of always offsetting it.
- **Random Texture** chooses a texture, randomizes its start behavior, and gives it an audible but
  controlled supporting level.
- **Random FX** explores the complete rack, character modes, and delay timing with moderated mixes.
- The large dice changes all unlocked sound layers and instrument settings, then adds only small,
  probability-based effect touches. Shimmer, Reverb, Drive, and Crush remain deliberately restrained.

Every parameter is exposed to the DAW for automation and saved with the FL Studio project.
Custom files are recalled by path, so keep them in place after saving a project.

## Development notes

See `GUI-GUIDE.md` for the interface map and quick standalone-preview workflow. The full
factory library is intentionally installed as external content rather than embedded into the
VST3; this keeps plugin scans and FL Studio startup fast.

JUCE has its own licensing terms. Review the current JUCE license before distributing or
selling compiled SIED builds. Only distribute audio for which you have the necessary rights.
