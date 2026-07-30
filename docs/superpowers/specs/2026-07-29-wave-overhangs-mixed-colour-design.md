# Wave overhangs × mixed colour: coexistence and ring-index coverage screening

**Date:** 2026-07-29
**Repo:** `sudharsan-gandhi/Orca-Flashforge`, branch `feat/wave-overhangs`
**Base:** Flash Studio `beta_hunse` @ `a4b96d6` (Orca 2.3.2) + WaveOverhangs v0.4.0 port
**Target machine:** FlashForge Creator 5 / Creator 5 Pro (4-toolhead tool changer)

## Goal

Let wave overhangs and mixed-colour filament be used on the same object — and
eventually the same surface — without destroying the print.

Delivered in two phases:

- **Phase 1 — safe coexistence.** Wave regions degrade to layer-granular colour.
  Small, shippable, removes both fatal mechanisms.
- **Phase 2 — ring-index coverage screening.** Wave rings become halftone screen
  elements, giving genuine sub-millimetre colour variation on overhang faces.

Phase 1 is a prerequisite for Phase 2, not a throwaway: it establishes the single
decision point that Phase 2 refines. Where Phase 1 pins a wave region to one filament
per layer, Phase 2 replaces that pin with ring-granular assignment inside the same
decision point — the layer-cycle path remains the fallback whenever screening is
disabled or the tool-change cap is zero. Phase 2 will get its own implementation plan;
this document specifies it only far enough to confirm Phase 1 does not foreclose it.

## The collision

Mixed colour has four mechanisms. Two are safe for wave paths, two are fatal.

| Mechanism | How it works | Wave safety |
|---|---|---|
| `LayerCycle` | One physical filament per **whole layer** (`MixedFilament.hpp:22`) | **Safe** — wave path stays continuous in one filament |
| `Simple` (default) | Per-layer cadence (`MixedFilament.hpp:24`) | Safe |
| `SameLayerPointillisme` | **Splits individual `ExtrusionPath`s** into segments with alternating extruders (`GCode.cpp:6124`) | **Fatal** |
| Local-Z dithering | Splits a layer into per-extruder **Z sub-passes** with runtime Z clipping | **Fatal** |

**Why pointillism is fatal:** it cuts one extrusion into per-extruder buckets. On a
cantilevered wave path that means the nozzle stops in mid-air, travels to the tool
dock, swaps, returns, and resumes on cooled unsupported material.

**Why Local-Z is fatal:** a wave ring anchors to the ring beside/below it at the
same Z. Shifting sub-layer Z per extruder bucket breaks that anchor relationship.

Confirmed defect: there is **no** `wave_overhang` guard anywhere near the
pointillism dispatch or the Local-Z bucketing. The two subsystems are mutually
unaware.

## Constraints that shape the design

### Colour physics: averaging, not subtractive

`filament_mixer.cpp:39` is a **pairwise `lerp`**, performed in linear light via
`srgb_to_linear`/`linear_to_srgb`. There are **zero** N-way blend functions in
`filament_mixer.cpp` or `filament_mixer_model.h`. The model is therefore linear-light
interpolation between two opaque colours — *reflectance averaging*.

Consequences, both load-bearing:

1. **No subtractive behaviour.** Averaging process C, M and Y in linear light gives
   sRGB ≈ `(205, 176, 165)` — a dusty beige, not black. CMYK's "key" is not a
   correction here; it would be the only dark primary available.
2. **Nothing can be lighter than the lightest loaded filament.** Real CMYK relies on
   white paper as its lightest primary. Omitting white forfeits every highlight.

Gamut is the convex hull of the loaded filament colours. With four toolheads,
**CMY + White** dominates CMYK for almost any model. If shadows are also needed,
white *and* black are required, leaving only two chroma slots — four-slot opaque
averaging is simply a small gamut, whichever basis is chosen.

*True* CMYK on FDM requires translucent colour over a white core so light transmits
and reflects (genuine subtraction, modulated by shell thickness). That is explicitly
out of scope here, and is incompatible with wave overhangs on the same surface: a
thin shell over a core presupposes a core, and a cantilever has none.

### Machine: tool changes cost 7 seconds

`machine_tool_change_time = 7` on the Creator 5 Pro. This, not path splitting, is
what bounds Phase 2. A 7 mm-deep wave layer at the default
`wave_overhang_line_spacing` of 0.35 mm is ~20 rings; screening every ring across
4 primaries is up to **20 × 7 s = 140 s** of tool changing on one layer, with the
cantilever tip oozing and cooling throughout. Arithmetic wall, not a fixable bug.

### Geometry: what is actually visible

On a true 90° overhang only the **bottom-most** wave layer is visible from below;
higher wave layers are hidden behind it. Screening on such a face is therefore
**1-D radial** — smooth transitions across the overhang's depth, not arbitrary
images. On an angled (e.g. 45°) overhang, successive layers present a staircase of
ring edges, giving genuine 2-D screening and much greater capability.

This is a capability boundary, not a defect. It should be documented for users.

## Phase 1 — safe coexistence

### Decision point

Resolve **once, at tool-ordering time, before wipe-tower preplanning.**

This placement is mandatory, not stylistic. `LocalZOrderOptimizer.hpp:11-12` states:

> Shared by wipe-tower preplanning and runtime Local-Z clipping. Both paths must
> classify the same pass extruders or their toolchange sequences diverge.

Guarding only at G-code emission would let preplanning and runtime disagree,
producing divergent tool-change sequences — on a 4-toolhead machine that means
purge volumes and tool-change G-code that do not match reality. Rejected for that
reason, despite being the smaller change.

### Data source

Reuse what the port already computes. `Layer` carries:

- `wave_overhang_floor_polygons` (`Layer.hpp:169`)
- `wave_overhang_covered_polygons` (`Layer.hpp:177`)
- `wave_overhang_shadow_polygons` (`Layer.hpp:186`)

populated in `LayerRegion.cpp:130-135` from the perimeter generator's
`out_wave_overhang_*` outputs, and settled before downstream stages
(`PrintObject.cpp:686`). `wave_overhang_covered_polygons` is the correct input: it
is written unconditionally wherever wave paths were generated.

### Behaviour

Per layer, intersect each region's area with `wave_overhang_covered_polygons`.
Where they overlap:

- Force that region's mixed filament to resolve in `LayerCycle` for that layer.
- Record the decision so preplanning, Local-Z bucketing and emission all read the
  same answer.

**Granularity is whole-region-per-layer, not sub-area.** If a region only partially
overlaps the wave footprint, the *entire* region degrades to `LayerCycle` for that
layer. Degrading only the overlapping sub-area would require splitting the region into
a separate `PrintRegion`, which was rejected as too invasive (it touches `.3mf`
compatibility, a hard constraint in `AGENTS.md`). The cost is that a region containing
even one wave path loses pointillism across its whole area on that layer; this is
accepted, and is why wave overhangs and fine colour work are best placed on separate
objects until Phase 2 lands.

Non-overlapping regions are untouched — full pointillism and Local-Z continue
everywhere else in the object. This is the approved behaviour: the overhang still
reads as the blended colour, because consecutive wave layers alternate filament.

### Thermal precedence

Inside wave paths, wave's `wave_overhang_nozzle_temp` and `wave_overhang_fan_speed`
**win** over the per-filament profile values. These overrides exist because
cantilevers are thermally fragile; losing the cantilever loses the print, whereas a
slightly off-nominal filament temperature does not.

### Material-type guard

Layer-cycle in a wave region means consecutive cantilever layers use *different
filaments*. If a mixed row's components are different material *types* (e.g. PLA and
PETG), the layer-to-layer bond in mid-air will be weak and the overhang can
delaminate.

Detect differing filament types among a mixed row's components inside a wave region
and fold it into the warning. **Warn, do not block** — same-material colour pairs are
the normal case and must stay frictionless.

### Warning surface

Silent auto-fix plus a **single non-blocking** notification describing what was
adjusted. This matches the existing precedent in `ConfigManipulation.cpp`, which
already handles the Local-Z vs `mixed_filament_region_collapse` conflict and carries
a one-time warning for Local-Z with variable layer height.

No new config option in Phase 1. The safe behaviour is the only correct behaviour;
an option to disable it would only let users produce broken prints.

## Phase 2 — ring-index coverage screening

### Principle

Adopt CMYK's *logic* — separate a target colour into primaries, then vary each
primary's **area coverage** below visual resolution — while rejecting CMYK's
primaries for the reasons above.

Wave overhangs already emit a natural screening structure. `WaveOverhangs::generate()`
returns `std::vector<ExtrusionPaths>`, one entry per wavefront
(`WaveOverhangs.cpp:83`) — **rings are already individually addressable**. Default
ring spacing 0.35 mm is below what the eye resolves at arm's length.

Assign **whole rings** to filaments. Never split a ring. A tool change then lands at
a ring boundary where the previous ring is a complete, self-anchored arc — nothing
dangles in mid-air, and wave already wants dwell (`wave_overhang_min_wave_time`).

### Components

1. **Separation.** Target colour → per-primary coverage fractions: a barycentric
   solve over the convex hull of the loaded filament colours in linear light. This is
   new code — the existing engine is pairwise only. The
   `dithering_local_z_direct_multicolor` option exists (`PrintConfig.cpp:2584`) but no
   carry-over/error-diffusion implementation was found in this base, so no reuse is
   available.
2. **Screening.** Error-diffuse the fractional coverage across ring index, so a
   sequence of whole-ring assignments integrates to the target fractions.
3. **Tool-change cap.** Hard limit on changes per wave layer, default **1–2**
   (7–14 s added dwell, plausibly within `wave_overhang_min_wave_time` tolerance).
   The screener must respect the cap, degrading colour resolution rather than
   exceeding it. Rings must print in propagation order — grouping by filament to save
   changes is **not** permitted, because each wavefront anchors to the previous.

### Ordering constraint

Wave propagation order is invariant. Any screening scheme that would reorder rings
is invalid regardless of how many tool changes it saves.

## Non-goals

- Colour variation *within* a single wave ring (the fatal mechanism, by design).
- Local-Z inside wave regions (geometrically unsound — a cantilever's anchor must
  share its Z).
- Translucent-over-white subtractive pipeline (separate project; see above).
- Re-tuning wave's fan/speed/temperature defaults for the Creator 5. Real work, but
  empirical tuning requiring the physical printer, not design.
- Arbitrary image reproduction on flat 90° undersides (geometrically limited to
  radial gradients).

## Testing

Phase 1:

- Unit: region/wave-footprint intersection returns expected overlap sets.
- Unit: a mixed row in pointillism mode resolves to `LayerCycle` for layers where its
  region overlaps `wave_overhang_covered_polygons`, and retains pointillism elsewhere.
- Invariant: preplanning and runtime classify identical pass extruders — directly
  targets the `LocalZOrderOptimizer.hpp:11-12` hazard.
- Regression: with `wave_overhangs` off, emitted G-code is byte-identical to the base.
- G-code inspection: no tool change occurs between the start and end of any path
  flagged `wave_overhang`.

Phase 2:

- Unit: separation solver returns weights summing to 1 and reproduces each basis
  colour exactly when the target equals that basis colour.
- Unit: error diffusion over N rings integrates to target coverage within tolerance.
- Invariant: tool changes per wave layer never exceed the configured cap.
- Invariant: ring emission order is unchanged by screening.

## Risks

1. **The port is not yet known to compile.** All of this sits on top of an unverified
   build. Phase 1 must not start until macOS CI is green.
2. **Tool-change dwell may harm cantilevers even at 1 change per layer.** 7 s of ooze
   and cooling mid-overhang is untested on this machine. Empirical; may force the cap
   to 0 for some geometries, which would reduce Phase 2 to layer-axis screening only.
3. **Gamut may disappoint.** Four-slot opaque averaging is a small gamut. Users
   expecting photographic colour will be unsatisfied regardless of implementation
   quality. Document the convex-hull limit prominently.
