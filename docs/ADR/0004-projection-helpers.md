# ADR 0004: Expose Basic Projection Helpers to GDScript

## Status

Accepted

## Context

`maplibre-native-godot` originally exposed only camera and style controls:

- `set_style_url`
- `fly_to`
- `jump_to`
- `set_pitch`
- `set_bearing`

That was enough for a pure "map as texture" demo, but not enough for
gameplay-layer overlays that need to align Godot-side geometry with the map.

The immediate driver is `MyGodotMapGame`, which uses `maplibre-native-godot`
as an API validation target. The game now wants to consume real GeoJSON parcel
data from Taito ward and render interactive regions on top of the live map
instead of using hand-authored normalized rectangles.

The underlying `maplibre-native` engine already exposes the required
projection primitives on `mbgl::Map`:

- `pixelForLatLng`
- `latLngForPixel`

The missing piece was a GDScript-facing wrapper.

## Decision

Expose two minimal projection helpers on `MapLibreMap`:

- `geo_to_screen(lat, lon) -> Vector2`
- `screen_to_geo(x, y) -> Dictionary { "lat", "lon" }`

These helpers are implemented as thin wrappers over `mbgl::Map` projection
methods inside `MapRuntime`.

## Consequences

Positive:

- Godot gameplay/UI code can align overlays to the rendered map without
  reimplementing map projection logic.
- Real GeoJSON-driven regions become practical.
- This keeps the API extension minimal while validating a concrete downstream
  need.

Tradeoffs:

- Per-point projection from GDScript is adequate for small overlays but may be
  too chatty for large datasets or per-frame heavy geometry updates.
- If downstream usage grows, a follow-up batch projection API may be warranted.

## Notes

This ADR does **not** add full source/layer styling APIs to GDScript.
It only adds the minimum projection surface needed for Godot-side overlay
experiments and gameplay validation.
