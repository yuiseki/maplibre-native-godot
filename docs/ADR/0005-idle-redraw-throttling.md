# ADR 0005: Add Idle Redraw Throttling for Large Static Scopes

## Status

Accepted

## Context

`maplibre-native-godot` started as a strictly continuous renderer:

- Godot `_process()`
- `MapRuntime::tick()`
- `renderOnce()`
- `readStillImage()`
- `ImageTexture::update()`

That is acceptable for small scopes and animated camera work, but it becomes
expensive for very large gameplay scopes where the map image is mostly static
and the interesting motion comes from Godot-side overlays.

`MyGodotMapGame` reached that point at the `prefectures` scope. Profiling
showed that after GDScript-side geometry work had been reduced, the dominant
remaining cost was still the native per-frame map render.

## Decision

Expose a minimal redraw policy on `MapLibreMap`:

- `set_idle_redraw_interval_ms(interval_ms)`
- `get_idle_redraw_interval_ms()`
- `request_redraw()`

Behavior:

- default remains `0`, meaning fully continuous redraw
- when the interval is greater than `0`, idle redraws are throttled
- camera/style/size changes still request immediate redraws
- `fly_to()` temporarily keeps continuous redraw during the animation window

This keeps lower scopes unchanged while allowing downstream games to reduce map
render frequency only for large, mostly static scopes.

## Consequences

Positive:

- Large scopes can trade map smoothness for much lower native render cost.
- Existing projects keep their old behavior unless they opt in.
- Animated camera transitions still look normal because they temporarily force
  continuous redraw.

Tradeoffs:

- The map texture may update less frequently than Godot overlays at large
  scopes.
- This is a policy knob, not a full invalidation-driven renderer. Downstream
  code still needs to choose sensible scope-specific intervals.

## Notes

This ADR intentionally does not introduce a full render scheduler. It only adds
the smallest API surface needed to validate large-scope gameplay in
`MyGodotMapGame`.
