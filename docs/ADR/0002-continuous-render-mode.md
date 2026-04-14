# ADR 0002: MapMode::Continuous with per-frame tick()

## Status

Accepted — validated

## Context

The initial render implementation used `MapMode::Static`, which calls
`frontend.render(map)` — a blocking call that waits for all tiles to load
before returning. This made `fly_to` appear as an instant teleport: each
GDScript call completed a full blocking render before Godot could repaint the
screen.

The goal is smooth animated map navigation identical to the arc-style fly_to
in `maplibre-native-slint`.

## Decision

Switch to `MapMode::Continuous` and drive the renderer from Godot's
`NOTIFICATION_PROCESS` (the per-frame callback):

- `MapRuntime` constructor creates `HeadlessFrontend` with
  `SwapBehaviour::Flush` (required by Continuous mode to present the
  framebuffer after each `renderOnce()` call).
- `MapRuntime::tick()` is called every frame:
  1. `run_loop.runOnce()` — pumps network/tile-load callbacks.
  2. `tick_animation()` — advances the fly_to animation state machine.
  3. `BackendScope` + `renderOnce()` + `readStillImage()` — renders one frame.
  4. `unpremultiply()` — converts premultiplied → straight alpha for Godot.
- `MapViewNode` calls `set_process(true)` in `NOTIFICATION_READY` and uploads
  new pixels via `ImageTexture::update()` (reuses GPU allocation every frame).

### fly_to animation state machine

Ported from `maplibre-native-slint`. Parameters:

| Parameter          | Value | Meaning                              |
|--------------------|-------|--------------------------------------|
| `duration_ms`      | 2500  | Total animation duration             |
| `mid_ratio`        | 0.60  | Zoom-out phase ends at 60% of time   |
| `center_hold_ratio`| 0.20  | Center stays near start for first 20%|
| `zoom_out_delta`   | 2–11  | Zoom-out depth, scaled by distance   |

All interpolation uses a cubic ease-in-out function.

## Consequences

### Positive

- Smooth, animated fly_to arcs visible every frame.
- `renderOnce()` is backend-agnostic — works on GL, Vulkan, Metal, and WebGPU.
- `readStillImage()` is implemented on all backends.
- Per-frame `ImageTexture::update()` avoids repeated GPU allocation.
- wgpu-native and Godot's renderer coexist without conflict (separate contexts).

### Negative

- CPU load is continuous (render every frame), not demand-driven.
- `readStillImage()` involves a CPU readback per frame; zero-copy texture
  sharing is left for a future milestone.

## Validation

Validated on Ubuntu 24.04 / Linux 6.x, RTX 3060, Godot 4.3:

- fly_to Paris / New York / Tokyo shows smooth arc animations.
- Benchmark over 90 frames shows consistent FPS.
- Pitch and bearing sliders update in real time.
