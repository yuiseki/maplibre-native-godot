#include "map_runtime.hpp"

#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/run_loop.hpp>

#include <cstring>
#include <exception>
#include <stdexcept>

namespace maplibre_godot {

// ---------------------------------------------------------------------------
// Impl — owns all maplibre-native objects
// ---------------------------------------------------------------------------

struct MapRuntime::Impl {
    mbgl::util::RunLoop    run_loop;   // must be constructed before frontend/map
    mbgl::HeadlessFrontend frontend;
    mbgl::Map              map;

    std::string last_style_url;

    Impl(uint32_t width, uint32_t height, float pixel_ratio)
        : run_loop()
        , frontend(mbgl::Size{width, height}, pixel_ratio)
        , map(frontend,
              mbgl::MapObserver::nullObserver(),
              mbgl::MapOptions()
                  .withMapMode(mbgl::MapMode::Static)
                  .withSize(mbgl::Size{width, height})
                  .withPixelRatio(pixel_ratio),
              mbgl::ResourceOptions::Default(),
              mbgl::ClientOptions())
    {}
};

// ---------------------------------------------------------------------------
// MapRuntime
// ---------------------------------------------------------------------------

MapRuntime::MapRuntime(uint32_t width, uint32_t height, float pixel_ratio)
    : impl_(std::make_unique<Impl>(width, height, pixel_ratio))
{}

MapRuntime::~MapRuntime() = default;

RenderResult MapRuntime::render(const std::string& style_url,
                                double lat,
                                double lon,
                                double zoom,
                                double bearing,
                                double pitch) {
    RenderResult result;
    try {
        // Reload style only when the URL changes
        if (style_url != impl_->last_style_url) {
            impl_->map.getStyle().loadURL(style_url);
            impl_->last_style_url = style_url;
        }

        // Update camera
        mbgl::CameraOptions camera;
        camera.center  = mbgl::LatLng{lat, lon};
        camera.zoom    = zoom;
        camera.bearing = bearing;
        camera.pitch   = pitch;
        impl_->map.jumpTo(camera);

        // Blocking render — HeadlessFrontend::render() drives the RunLoop
        // internally until the still image callback fires.
        auto render_result = impl_->frontend.render(impl_->map);
        const auto& img    = render_result.image;

        if (!img.valid()) {
            result.error   = "render returned an invalid image";
            result.success = false;
            return result;
        }

        result.width   = img.size.width;
        result.height  = img.size.height;
        result.pixels.resize(img.bytes());
        std::memcpy(result.pixels.data(), img.data.get(), img.bytes());
        result.success = true;
    } catch (const std::exception& e) {
        result.error   = e.what();
        result.success = false;
    } catch (...) {
        result.error   = "unknown error during render";
        result.success = false;
    }
    return result;
}

void MapRuntime::resize(uint32_t width, uint32_t height) {
    impl_->frontend.setSize(mbgl::Size{width, height});
}

} // namespace maplibre_godot
