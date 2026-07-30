# Flash Studio + WaveOverhangs

A slicer for the **FlashForge Creator 5 / Creator 5 Pro** that combines two things
no single upstream build has together:

- **Full-spectrum colour mixing** — create new colours by alternating physical
  filaments layer-by-layer (yellow + blue reads as green), with gradients,
  manual layer patterns and Local-Z dithering.
- **Support-free wave overhangs** — print steep and 90° overhangs as curved
  wavefronts cantilevered into open air, instead of building support structures
  under them.

## What this fork is

| | |
|---|---|
| Base | [`FlashForge/Orca-Flashforge`](https://github.com/FlashForge/Orca-Flashforge) branch `beta_hunse` @ `a4b96d6` — the 1.7.9-beta "混色" (mixed-colour) branch, Orca 2.3.2 |
| Colour mixing | FlashForge's engine, **unmodified** (`MixedFilament.cpp` is byte-identical to the base) |
| Printer support | FlashForge's Creator 5 + Creator 5 Pro profiles, **unmodified** |
| Added here | Wave overhangs, ported from [`dennisklappe/OrcaSlicer-WaveOverhangs`](https://github.com/dennisklappe/OrcaSlicer-WaveOverhangs) v0.4.0 (Orca 2.4.0-dev) |

Why a fork at all: FlashForge ship the colour mixing and the Creator 5 profiles
but no wave overhangs; the WaveOverhangs fork tracks mainline OrcaSlicer and has
neither. Combining them requires back-porting wave from Orca 2.4.0-dev onto the
2.3.2 base FlashForge builds from.

The Creator 5 Pro is a 4-toolhead tool-changer (`nozzle_diameter` ×4,
`extruder_offset` all `0x0`, empty `change_filament_gcode` — the tool change is a
bare `T<n>` handled in firmware), which is what makes per-layer filament
alternation practical on it.

## Using wave overhangs

**Print Settings → Wave overhangs → General → "Use wave overhangs (Experimental)"**.
Off by default; with it off, behaviour matches stock Flash Studio.

- **Per-object.** It is a `PrintRegionConfig` option, so right-click any object (or
  a modifier volume) to override it. One model can use waves while another uses
  supports on the same plate.
- **Supports still work alongside it.** `Support unfilled wave overhang areas`
  (default **on**) generates supports only for overhangs the waves did not cover.
- `Use wave overhangs instead of bridges` (default off) — leave it off and flat
  spans stay as ordinary bridges; only concave or holed overhangs get waves.
- The page is organised General → Detection → Pattern → Corner reinforcement →
  Motion & Cooling → Floor layers → Debug. Every tunable is hidden while the
  master toggle is off.

## Using colour mixing

Load two or more filaments and a **Mixed Colors** panel appears in the sidebar
with every available combination. Assign a mixed filament to an object like any
physical one; it resolves to alternating layers at slice time. Ratio, per-pair
bias, gradients and Local-Z dithering are under
**Print Settings → Others → Mixed Filaments** and **Others → Dithering**.

## ⚠️ Do not combine both on the same region yet

Colour mixing alternates filament per layer. Wave paths are cantilevered into
open air and carry their own speed, fan and nozzle-temperature overrides
precisely because they are thermally fragile. Where a mixed-colour region
overlaps a wave-overhang region, a **tool change can land mid-cantilever** and
both subsystems' overrides compete for the same paths.

This is unresolved. Use the two features on different objects or different
regions until it has been characterised. Neither upstream project has this
interaction either — it only arises once they are combined.

Also: nobody has tuned wave's fan/speed/temperature defaults for the Creator 5.

## Install

Builds are unsigned — signing and notarisation need Apple credentials this fork
does not have. On macOS the app is blocked on first launch:

```console
xattr -dr com.apple.quarantine /Applications/OrcaSlicer.app
```

Or right-click the app → **Open** → **Open**.

## How to compile

- Windows 64-bit
  - Tools needed: Visual Studio 2022, CMake, Git, Strawberry Perl.
  - Run `build_release.bat` in `x64 Native Tools Command Prompt for VS 2022`

- Mac 64-bit
  - Tools needed: Xcode, CMake, Git, gettext, Automake, Perl
  - run `build_release_macos.sh`

A full build including dependencies needs roughly 30 GB of free disk.

## What changed relative to `beta_hunse`

The wave patch was derived from wave's own commits only — diffed against its
merge-base with upstream OrcaSlicer (`3e4af2c`, 2026-04-13) — so none of the
2.4-era upstream drift came along. 29 files, +3291/−37.

Deliberately **not** taken from upstream wave:

- its rebranding commits, which would have renamed Flash Studio to "OrcaSlicer"
- an unrelated `ConfigImport`/`ConfigImportPrusa` feature (~740 lines) that wave
  had itself cherry-picked from upstream

Two resolutions worth knowing about:

- **The G-code header is left as Flash Studio's.** Upstream wave rewrites it to
  `OrcaSlicer 2.4.0-WaveOverhangs`. FlashForge firmware is specifically fussy
  about header contents — wave issue #74 was a FlashForge AD5X reporting
  "No filament detected" over exactly this — so the header the Creator 5
  firmware is tested against is untouched.
- **`CoolingBuffer.cpp` needed a real back-port.** Wave targets the 2-argument
  `GCodeWriter::set_fan`; this base takes a third `part_cooling_fan_min_pwm`
  argument. Wave's fan-override branch was rewritten against the 2.3.2 signature.

Full conflict-by-conflict detail is in
[PR #1](https://github.com/sudharsan-gandhi/Orca-Flashforge/pull/1).

## Credits

Wave overhangs:

- Algorithm: **Janis A. Andersons** (andersonsjanis)
- Builds on the arc-overhang algorithm by **Steven McCulloch** (stmcculloch),
  who also did the PrusaSlicer integration
- OrcaSlicer port: **Dennis Klappe** (dennisklappe)

Colour mixing comes from FlashForge's `beta_hunse` branch. Substantially the same
`MixedFilament.cpp` also appears in
[`ratdoux/OrcaSlicer-FullSpectrum`](https://github.com/ratdoux/OrcaSlicer-FullSpectrum)
and in Snapmaker's OrcaSlicer fork. Colour blending is powered by
[FilamentMixer](https://github.com/justinh-rahb/filament-mixer), an openly
licensed library.

# License

This fork is licensed under the GNU Affero General Public License, version 3. It
is based on Orca-Flashforge by FlashForge and incorporates work from
OrcaSlicer-WaveOverhangs by Dennis Klappe, which is also licensed under the GNU
Affero General Public License, version 3.

Orca-Flashforge is licensed under the GNU Affero General Public License, version 3. Orca-Flashforge is based on Orca Slicer by SoftFever.

Orca Slicer is licensed under the GNU Affero General Public License, version 3. Orca Slicer is based on Bambu Studio by BambuLab.

Bambu Studio is licensed under the GNU Affero General Public License, version 3. Bambu Studio is based on PrusaSlicer by PrusaResearch.

PrusaSlicer is licensed under the GNU Affero General Public License, version 3. PrusaSlicer is owned by Prusa Research. PrusaSlicer is originally based on Slic3r by Alessandro Ranellucci.

Slic3r is licensed under the GNU Affero General Public License, version 3. Slic3r was created by Alessandro Ranellucci with the help of many other contributors.

The GNU Affero General Public License, version 3 ensures that if you use any part of this software in any way (even behind a web server), your software must be released under the same license.

Orca-Flashforge includes a pressure advance calibration pattern test adapted from Andrew Ellis' generator, which is licensed under GNU General Public License, version 3. Ellis' generator is itself adapted from a generator developed by Sineos for Marlin, which is licensed under GNU General Public License, version 3.

The flashforge networking plugin is based on non-free libraries from FlashForge. It is optional to the Orca-Flashforge and provides extended functionalities for FlashForge printer users.
