#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace maplibre_godot {

struct RenderResult {
    std::vector<uint8_t> pixels; // RGBA, premultiplied alpha, row-major
    uint32_t width   = 0;
    uint32_t height  = 0;
    bool     success = false;
    std::string error;
};

/// Persistent headless maplibre-native renderer.
/// Owns a RunLoop, HeadlessFrontend, and Map instance.
/// Must be created and used on the same thread.
class MapRuntime {
public:
    MapRuntime(uint32_t width, uint32_t height, float pixel_ratio = 1.0f);
    ~MapRuntime();

    /// Render a still image.  Blocks until the frame is complete.
    RenderResult render(const std::string& style_url,
                        double lat,
                        double lon,
                        double zoom,
                        double bearing,
                        double pitch);

    /// Update the render target dimensions.
    void resize(uint32_t width, uint32_t height);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace maplibre_godot
