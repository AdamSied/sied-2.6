# Changelog

## 2.6.0

- Simplified the complete interface with a lighter warm-neutral palette, flatter surfaces, quieter
  reactive accents, fewer ornaments, and clearer spacing.
- Removed the header symbol and deleted Seed/Variation history and controls.
- Added a focused 32-step randomization Undo history that restores parameters, layers, paths, and
  sound selections after one-shot, texture, FX, or complete-patch randomization.
- Rebuilt root detection to analyse several non-overlapping stable frames and reject inconsistent,
  low-confidence, noisy, or polyphonic results.
- Added automatic per-layer cents correction and preserved that correction through randomization.
- Verified the stricter detector against all 15 embedded instruments with the documented root
  octave matched in every case.

## 2.5.0

- Rebuilt the visual language around a neutral, precise hierarchy with flatter surfaces, cleaner
  spacing, stronger waveform focus, restrained activity light, and functional labels only.
- Removed the Aether/engine/readiness copy, topographic decoration, FX numbering, and text from the
  main dice control.
- Added a dedicated searchable Library page with source/category filters, destination selection,
  persistent favorites, row-level star controls, and double-click loading.
- Added 18 automatable randomization locks for layers, Envelope, Shimmer, Voice, and every FX module.
- Added deterministic seed history with previous, next, and variation actions (removed in 2.6).
- Added two captured scenes and a continuous A/B parameter morph with discrete sample switching.
- Added independent slot A/B root parameters and automatic YIN-style pitch detection with confidence
  reporting for imported one-shots.
- Preserved the corrected 226-one-shot and 113-texture external factory library unchanged.

## 2.4.0

- Rebuilt the complete interface as the SIED Aether visual system: machined controls,
  illuminated value capsules, topographic signal texture, dimensional browser and FX surfaces,
  animated status details, and a restrained use of the original collage.
- Replaced the waveform renderer with filled, layer-coloured source displays and smoother,
  brighter playback markers.
- Fixed playback-marker reporting for short samples, note endings inside an audio block, and
  polyphonic playback by tracking each rendered block and preferring the newest active note.
- Changed full-preset Release randomization to operate in knob-travel space: 80% of results are
  between 10-40%, 16% are between 40-70%, and 4% are between 70-95%.
- Added `SIED-FUTURE-IDEAS.md`, a non-implemented design roadmap for future instrument features.

## 2.3.0

- Rebuilt the visual system with dimensional controls, deeper panels, cleaner typography,
  refined waveforms, and value-reactive glow around Shimmer and every FX card.
- Added linked nonlinear Amount/Mix response curves so low FX settings stay subtle while
  preserving the complete range at higher settings.
- Retuned full-preset and dedicated FX randomization for restrained, balanced ambience.
- Raised randomized texture levels so the texture layer remains clearly audible.
- Added persistent user-preset saving and loading for parameters, sound selections, and paths.
- Added sound-name fallback so user presets remain portable when a factory library moves.

## 2.2.1

- Changed full-preset randomization to leave Shimmer and every other effect fully off.
- Reset all FX amounts, mixes, modes, and effect memory when using the header `RANDOMIZE` action.
- Confirmed the bundled factory bank is exclusively the replacement library from the three new uploads.

## 2.2.0

- Added live playback markers to all three sample waveforms.
- Added previous/next navigation for each sound slot.
- Added automatable dry/wet Mix controls for all 13 effects and rebalanced their sweeps.
- Increased Shimmer's maximum octave/pad response while double high-passing its reverb feed.
- Changed startup and INIT to `key - topograph`, slot B off, and the `Forest` texture.
- Added a cleaner `RANDOMIZE` dice action and expanded Random FX coverage.
- Retuned full-preset randomization to restrain Shimmer and favor 5–15% Attack positions.
- Replaced the external factory bank with 226 corrected one-shots and 113 textures.

## 2.1.1

- Added an Inno Setup definition for a standard Windows VST3 and factory-library installer.
- Added one-command Windows build-and-package support and automatic optional Inno Setup install.
- Added strict native-command exit-code checks and final VST3 bundle validation.
- Fixed the generated `DejaVuSansMonoBold_ttf` binary-data identifier on Windows.
- Switched MSVC builds to the static runtime for easier distribution to clean Windows systems.
- Added SHA-256 output after installer creation.
- Added full and plugin-only macOS `.pkg` builders for universal Apple Silicon/Intel releases.
- Added a manually triggered Mac cloud build that requires no development tools on the user's Mac.
- Added shared Mac factory-library scanning with user-library precedence and duplicate suppression.

## 2.1.0

- Added smoothed ON/OFF controls for One-shot A, One-shot B, and Texture.
- Added independent ±24-semitone transpose and ±100-cent fine tune per layer.
- Added a complete INIT patch reset with default sound reloading and cleared effect memory.
- Added FL Studio/host tempo-synced delay divisions, dotted/triplet values, and free millisecond time.
- Added Chorus, Crush, Drive, Phaser, Flanger, Tremolo, and Compressor character selectors.
- Added smooth fractional delay-time changes to avoid clicks when switching divisions.
- Changed one-shot randomization so each layer keeps its original start point half the time.
- Embedded a modern monospace value font and redesigned popup menus to match the dark interface.
- Replaced the old version subtitle with `adamsied`.

## 2.0.0

- Added two one-shot layers and one always-looping texture layer.
- Added the supplied 234-one-shot and 87-texture factory library.
- Added automatic peak normalization and visually normalized waveforms.
- Added separate one-shot, texture, FX, and full-preset randomizers with safe gain/effect ranges.
- Added click-free equal-power loop wrapping and longer texture crossfades.
- Added independent one-shot start/end regions and texture random start.
- Added 1–32 voice polyphony, Mono, and Glide.
- Added delay, reverb, low-pass, and high-pass mode selectors.
- Moved Shimmer to the front page and refined its tuned octave/reverb signal path.
- Rebuilt the interface and browser around a minimal three-layer workflow.
- Added automatic factory-library installation to both platform build scripts.

## 1.0.0

- Added waveform start/end editing, reverse, loop modes, 15 embedded sounds, and the full FX page.
