# SIED 2.6 GUI guide

Visual work is isolated from the audio engine:

- `Source/PluginEditor.cpp` draws and positions the interface.
- `Source/PluginEditor.h` declares its controls.
- `SiedLookAndFeel::drawRotarySlider` is the complete knob design.

## Fast preview

After changing the GUI, build only the standalone target from Developer PowerShell:

```powershell
cmake --build build --config Debug --target SIED_Standalone
```

Then open `build\SIED_artefacts\Debug\Standalone\SIED.exe`. Use `build-vs2026` instead of
`build` if that is the folder you configured manually.

## Interface map

| Goal | Location in `PluginEditor.cpp` |
| --- | --- |
| Palette | Color constants at the top |
| Modern arc knobs | `SiedLookAndFeel::drawRotarySlider` |
| Consistent buttons and dice/arrows | `drawButtonBackground`, `drawButtonText` |
| Consistent menus | `drawComboBox`, font overrides |
| Value typography | Embedded `DejaVuSansMono-Bold.ttf` and `valueFont` |
| Matching dropdown popup | `SiedLookAndFeel::drawPopupMenuItem` and `drawPopupMenuBackground` |
| Value capsules and tooltips | `SiedLookAndFeel::drawLabel` and `drawTooltip` |
| Waveforms and live playheads | `SampleWaveformComponent` |
| Search, filters, favorites, and sound list | `LibraryBrowserComponent` |
| Page/card drawing | `SiedAudioProcessorEditor::paint` |
| Control positions | `SiedAudioProcessorEditor::resized` |
| Names, colors, tool behavior | Editor constructor |
| Initial size and resize limits | Editor constructor |

JUCE colors use `0xffRRGGBB`. The 2.6 interface uses a lighter warm-neutral surface system with
desaturated seafoam, lavender, and coral reserved for functional grouping and value feedback. It
intentionally avoids wallpaper, fake status messages, ornamental meters, and fictional engine names.

The SAMPLE page is intentionally hierarchical: browser first, three instrument cards second,
Shimmer in the center, and envelope/voice controls around it. Each card contains power, a
randomization lock, level, transpose, fine tuning, a layer-coloured waveform, and playback
feedback. The FX page uses a six-column matrix with value-reactive outlines and per-module locks;
every named effect has a paired Mix knob, with its type or timing menu directly below the related
control. The Library page is a dedicated high-density search view with persistent favorites. The
header contains the preset browser, save action, A/B scene morph, randomization Undo, INIT, and a
large icon-only dice action.
