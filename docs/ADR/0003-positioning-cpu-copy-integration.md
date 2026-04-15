# ADR 0003: Position the project as a practical CPU-copy integration

## Status

Accepted — validated

## Context

This project exists because we want to build casual, map-centric games in
Godot with `maplibre-native`.

The current technical situation is asymmetric:

- `maplibre-native` is built here with its WebGPU / `wgpu-native` headless path.
- Godot uses its own renderer stack (`Forward Plus` on desktop, with Vulkan on
  Linux and Metal on macOS) and currently does not expose a WebGPU-based path
  for this integration.
- The practical integration path available today is:

  ```text
  maplibre-native (wgpu/WebGPU backend)
    -> renderOnce()
    -> readStillImage()
    -> CPU-side copy
    -> Godot ImageTexture::update()
  ```

This is not a zero-copy design. However, it is the path that currently matches
Godot's renderer architecture, GDExtension boundaries, and texture ownership
model without requiring deep engine patches.

The strategic constraint matters too: `maplibre-native` is increasingly
WebGPU-oriented, while Godot is not centered on WebGPU. Chasing true
cross-platform zero-copy sharing inside Godot would therefore require backend-
specific interop work at exactly the layer where the two projects diverge.

## Decision

Position `maplibre-native-godot` as a **practical Godot integration** rather
than a zero-copy rendering experiment.

Concretely, this means:

- Keep the current CPU-copy path as the primary implementation.
- Treat Full HD / fixed internal resolution as the main performance target.
- Treat Godot's own resolution scaling and upscaling features as part of the
  intended operating model.
- Optimize for casual / indie / map-centric games, prototypes, and
  visualizations.
- Do **not** position this repository as the path for 4K high-refresh or
  zero-copy shared-texture workloads.

## Consequences

### Positive

- The repository solves a real problem today: showing live MapLibre maps inside
  Godot with smooth `MapMode::Continuous` animation.
- The design works across Linux, Windows, and macOS without requiring backend-
  specific GPU sharing code.
- The repo is aligned with a realistic use case: many Godot games are
  resolution-scaled, casual, or stylistically simple enough that Full HD is an
  acceptable internal render target.
- The project remains useful even if true zero-copy interop never becomes
  practical in Godot.

### Negative

- CPU readback and re-upload remain part of every frame.
- Performance degrades significantly at 4K-class resolutions because the main
  bottleneck is memory transfer, not map rendering alone.
- This repository is not the right place to demonstrate a “shared WebGPU
  texture everywhere” story comparable to `maplibre-native-slint`.

## Validation

Validated in the current implementation and documentation:

- `maplibre-native` is built with the WebGPU / `wgpu-native` backend.
- Godot integration uses `renderOnce()` + `readStillImage()` + `ImageTexture::update()`.
- Smooth interaction is practical at Full HD on modern machines.
- 4K-class rendering shows the expected CPU-copy bottleneck on both macOS and
  Windows.
