# ADR 0001: Linux PoC build pipeline

## Status

Accepted

## Context

We want a realistic path toward `maplibre-native-godot`, but the current
constraint set is hostile:

- `maplibre-native` is increasingly centered on WebGPU-oriented paths.
- Godot desktop integration through GDExtension is viable, but direct GPU
  resource sharing is not the right first step.
- Existing Godot map plugins in local forks are too old to guide a modern
  implementation.

The immediate need is not a polished renderer. The immediate need is a build
pipeline that makes the relationship between:

- the Godot project,
- the GDExtension,
- and the local `maplibre-native` fork

clear and reproducible.

## Decision

For the Linux PoC, use a two-stage build:

1. Build `maplibre-native` separately into a local build directory inside this
   PoC repository.
2. Build a thin C++ GDExtension against that result.

The GDExtension remains a stub at first. It should link cleanly and register a
Godot class before we attempt headless rendering or texture upload.

## Consequences

### Positive

- The seam between Godot and `maplibre-native` is explicit.
- Failures are easier to localize.
- We avoid coupling the first PoC to Godot renderer internals.
- We can iterate on rendering later without redesigning the build.

### Negative

- The pipeline is not yet convenient for end users.
- `godot-cpp` is still an explicit dependency.
- The first PoC does not prove rendering, only build coherence.
- Runtime loading semantics still need validation outside editor mode.

## Follow-up

Once the build pipeline is stable, the next step is:

1. create a `MapRuntime` wrapper around headless `maplibre-native`,
2. render to CPU image data,
3. upload that image into a Godot texture,
4. only later investigate native texture sharing.

## Reality check

As of the current implementation:

- the Linux build pipeline is working,
- the Godot editor initializes the extension,
- but headless scene startup still fails to resolve `MapLibreMap`.

So this ADR is now validated as a build-pipeline decision, not yet as a
runtime-integration decision.
