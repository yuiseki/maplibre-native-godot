#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace maplibre_godot {

struct RenderResult {
    std::vector<uint8_t> pixels; // RGBA, straight alpha (unpremultiplied)
    uint32_t width   = 0;
    uint32_t height  = 0;
    bool     updated = false; // true when pixels contain a newly rendered frame
    bool     success = false;
    std::string error;
};

/// Continuous-mode headless maplibre-native renderer.
///
/// Call tick() once per Godot _process() frame.
/// tick() pumps the RunLoop, advances the fly_to animation, renders one
/// frame (renderOnce), and reads back the pixels (readStillImage).
class MapRuntime {
public:
    MapRuntime(uint32_t width, uint32_t height, float pixel_ratio = 1.0f);
    ~MapRuntime();

    // --- Per-frame driver ---
    // Call from NOTIFICATION_PROCESS every frame.
    // Returns the rendered frame; result.updated = false on error / no change.
    RenderResult tick();

    // --- Style ---
    void set_style_url(const std::string& url);

    // --- Camera (instant) ---
    void jump_to(double lat, double lon, double zoom,
                 double bearing = 0.0, double pitch = 0.0);
    void set_pitch(double pitch);
    void set_bearing(double bearing);

    // --- Camera (animated 2.5 s arc, ported from maplibre-native-slint) ---
    // current_zoom_hint: caller's tracked zoom, used as fallback when
    // getCameraOptions().zoom returns nullopt (observed on Windows/wgpu-Vulkan).
    void fly_to(double lat, double lon, double zoom, double current_zoom_hint = 1.0);

    // --- Resize render target ---
    void resize(uint32_t width, uint32_t height);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace maplibre_godot
