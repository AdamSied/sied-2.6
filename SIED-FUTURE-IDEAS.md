# SIED future ideas

These are design directions that remain unimplemented after SIED 2.6. Random locks,
randomization Undo, A/B morphing, guarded root detection, search, and favorites are already in the
instrument.

## Best next moves

1. **Four assignable performance macros** — `AIR`, `MOTION`, `WEIGHT`, and `DISTANCE` by default,
   with user assignment. Each macro could move several existing parameters through carefully
   chosen ranges.
2. **Mood tags** — Airy, Dark, Organic, Glass, Soft, Broken, Warm, and Wide, with editable local
   metadata and smart suggestions.

## Signature sound engines

- **Bloom** — separate a one-shot's transient from its sustain, stretch the sustain into a pad,
  and crossfade from hit to atmosphere.
- **Spectral hold** — freeze a tiny harmonic snapshot at the current playhead and let it become an
  endless, clean pad behind the source.
- **Grain veil** — turn the texture slot into a restrained granular cloud with Density, Size,
  Scatter, Pitch, and Shape. Musical defaults matter more than extreme ranges.
- **Constellation playback** — define several start points on a sample and drift among them in a
  repeatable pattern rather than using ordinary random start.
- **Harmonic shadow** — generate quiet fifth, octave, sub-octave, or scale-aware companion voices
  from either one-shot.
- **Resonance capture** — listen for the strongest partials in a sound and build a tuned resonator
  bank that can ring after the original sample.
- **Texture imprint** — use the spectrum or amplitude of the texture to animate the one-shots, so
  rain, vinyl, crowds, and wind physically shape the instrument.
- **Transient ghost** — repeat only the first few milliseconds of a one-shot as a soft rhythmic
  halo while the main body plays normally.
- **Infinite loop finder** — scan a sample and suggest the smoothest loop points, then audition the
  best three candidates.
- **Reverse bloom** — synthesize a click-free reverse lead-in before each note while keeping the
  actual one-shot forward and punchy.
- **Formant drift** — move vocal or organic samples through gentle formant changes without simply
  transposing them.
- **Stereo particles** — extract tiny high-frequency details and place them around the stereo
  field as a controlled sparkle layer.

## Motion and modulation

- **Motion lanes** — two drawable curves that can loop freely or sync to tempo, then drag their
  handles onto any knob.
- **Step animator** — a compact 8/16-step lane for rhythmic volume, pan, filter, start position,
  shimmer, or effect mix.
- **Chaos with memory** — controlled wandering that returns toward a centre value instead of
  producing harsh white-noise modulation.
- **Note-age modulation** — make parameters change according to how long a note has been held;
  for example, texture and Shimmer slowly emerge after the initial transient.
- **Velocity maps** — choose whether velocity changes level, brightness, attack, sample start, or
  layer blend per slot.
- **Key tracking matrix** — allow higher notes to become shorter/brighter and lower notes to gain
  more body or texture.
- **Envelope follower** — let one layer's loudness animate another layer or an effect.
- **Per-note drift** — give each polyphonic voice small independent tuning, pan, start, or texture
  differences for a living hardware feel.
- **Gesture recorder** — record a few seconds of knob movement, loop it, and scale its depth.
- **Probability gates** — give each layer a chance to sound on each note, with a musical streak
  limiter so randomness never creates long accidental silences.
- **Euclidean pulse** — a minimal pulse generator for texture or transient repeats, useful for
  ambient trap without turning SIED into a full sequencer.

## Smarter randomization

- **Intensity control** — `Safe`, `Fresh`, and `Wild` determine how far randomization moves from
  the current patch.
- **Mutate amount** — create a nearby variation at 5–50% difference instead of a full reset.
- **Vibe targets** — Airy, Dark, Organic, Frozen, Nostalgic, Hollow, Dreamy, and Broken would use
  curated parameter rules rather than arbitrary values.
- **Role targets** — randomize specifically for Key, Pad, Pluck, Bell, Lead, Bass, or Texture Bed.
- **Audition history** — automatically retain the last 20 randomized patches until the plugin is
  closed, with a heart button to save any of them.
- **Smart gain matching** — loudness-match every new result to the previous result so better does
  not merely sound louder.
- **Clash detection** — notice when both one-shots occupy the same pitch/brightness space and
  adjust octave, tone, pan, or level to separate them.
- **Effect budget** — randomization receives a total wetness budget and distributes it between
  effects, preventing several individually safe settings from becoming overwhelming together.
- **Context-aware release** — choose shorter tails for plucks and longer tails for pads, while
  still respecting the user's selected release range.

## Sample and library workflow

- **Waveform audition keyboard** — click a sample name and hear a short preview at the current MIDI
  note without loading it permanently.
- **Similarity browser** — select a sound and show nearby sounds by brightness, length, pitch,
  noisiness, and envelope. Analysis could remain fully offline.
- **Visual sound map** — place the library in a 2D field from Dark to Bright and Organic to
  Synthetic; clicking anywhere auditions that region.
- **Automatic categories** — classify imported sounds, then let the user correct tags without
  renaming files.
- **Collections** — user-made folders that reference samples without duplicating the audio.
- **Missing-file repair** — when a preset's sample moved, search by name, duration, and audio hash
  and offer the closest match.
- **Pack installer** — a `.siedpack` file containing audio, artwork, metadata, categories, roots,
  and presets, installed by double-clicking or dropping it onto SIED.
- **Library health page** — identify duplicates, missing roots, clipped audio, silence, corrupt
  files, and unusually loud textures.
- **Auto-trim and fade** — detect leading silence, locate the transient, and add tiny reversible
  fades to imported sounds.
- **Batch tuning assistant** — analyse a folder, show root-note suggestions in a table, and let the
  user approve or correct everything before import.
- **Embedded preview cache** — generate tiny waveform/analysis files so a huge library opens
  instantly without rescanning every audio file.

## Performance and composition

- **Chord memory** — store several user chords and trigger them from one note with scale-safe
  voice leading.
- **Strum cloud** — spread chord notes in time, velocity, start position, and stereo placement.
- **Scale guard** — constrain harmonic shadows, pitch randomization, and chord tools to the chosen
  key without altering normal MIDI input.
- **Latch and evolve** — hold a chord, then let motion lanes and texture evolve while the hands are
  free for other controls.
- **Scene pads** — eight snapshots per preset for live switching or morphing, with MIDI learn.
- **Smart unison** — add width and life through independent sample starts and microtuning, with
  mono-safe control and predictable gain compensation.
- **MPE expression** — per-note pressure could increase Shimmer, slide could move sample position,
  and timbre could blend layers.
- **MIDI drag-out phrases** — a tiny optional idea generator could create rhythmically sparse
  ambient chord phrases and export ordinary MIDI to the DAW.
- **One-key transitions** — generate a reversible swell, impact, or texture tail from the active
  patch for arrangement transitions.

## Effects and spatial design

- **Effect routing view** — drag modules into serial or parallel lanes while keeping the normal FX
  page simple by default.
- **Send-style Shimmer** — choose whether Shimmer receives dry sound, Reverb output, Delay output,
  or only the texture layer.
- **Spectral reverb ducking** — reduce only the frequencies where the dry sound is active, keeping
  a huge tail without losing clarity.
- **Transient-safe ambience** — automatically preserve the attack while Delay/Reverb/Shimmer grow
  behind it.
- **Freeze feedback** — capture Delay or Reverb into an infinite bed with a safe limiter and a
  click-free release.
- **Distance model** — one macro combines early reflections, filtering, stereo narrowing, level,
  and pre-delay so a sound moves backward realistically.
- **Motion reverb** — slowly move the reverb field independently from the dry source.
- **Texture-only FX lane** — a small dedicated chain for the texture so it can be filtered and
  widened without changing the main instrument.
- **Sidechain input** — allow external audio or the DAW kick to duck texture and ambience.
- **Adaptive headroom** — measure the combined layers/effects and apply transparent gain control
  before clipping, with a visible amount meter.

## Visual and identity ideas

- **Living waveform** — spectral colour inside the waveform shows brightness while the playhead
  leaves a short fading trail.
- **Patch cover cards** — user presets can optionally store a colour and a small generated visual,
  giving the browser a recognizable identity without requiring artwork.
- **Signal-flow focus** — hover a knob to illuminate the part of the visual signal path it affects.
- **Macro constellations** — thin lines show which parameters are assigned to each macro and move
  as the macro changes.
- **Quality modes** — Eco, Studio, and Render clearly communicate oversampling/granular quality and
  CPU cost.
- **Accessible palettes** — optional colour-blind-safe accents and scalable text while preserving
  the main SIED look.
- **Minimal/performance view** — collapse the interface to the three layers, four macros, Shimmer,
  and preset navigation for recording or live use.

## A sensible development order

The strongest practical sequence would be: Randomize Locks and History, automatic root detection,
search/favorites, A/B Scene Morph, four macros, then Grain Veil or Spectral Hold as the first new
sound engine. That order improves everyday use before adding heavier DSP, while the Scene Morph
and spectral/granular engine give SIED a feature people can immediately recognize as its own.
