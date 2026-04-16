#include "map_runtime.hpp"
#include "animation.hpp"

#include <mbgl/gfx/backend_scope.hpp>
#include <mbgl/gfx/headless_frontend.hpp>
#include <mbgl/map/camera.hpp>
#include <mbgl/map/map.hpp>
#include <mbgl/map/map_observer.hpp>
#include <mbgl/map/map_options.hpp>
#include <mbgl/map/mode.hpp>
#include <mbgl/storage/resource_options.hpp>
#include <mbgl/style/style.hpp>
#include <mbgl/util/client_options.hpp>
#include <mbgl/util/premultiply.hpp>
#include <mbgl/util/run_loop.hpp>

#include <chrono>
#include <cstring>
#include <exception>

namespace maplibre_godot {

// ---------------------------------------------------------------------------
// Animation helpers (ported from maplibre-native-slint)
// ---------------------------------------------------------------------------

struct AnimState {
    bool     active            = false;
    mbgl::LatLng start_center  = {0, 0};
    mbgl::LatLng target_center = {0, 0};
    double start_zoom          = 0.0;
    double target_zoom         = 0.0;
    double mid_zoom            = 0.0;
    double mid_ratio           = 0.60;   // 60 % zoom-out, 40 % zoom-in
    double center_hold_ratio   = 0.20;
    std::chrono::steady_clock::time_point start_time;
    int    duration_ms         = 2500;
};

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct MapRuntime::Impl {
    mbgl::util::RunLoop    run_loop;
    mbgl::HeadlessFrontend frontend;
    mbgl::Map              map;
    AnimState              anim;
    std::string            last_style_url;

    static constexpr double kMinZoom = 0.0;
    static constexpr double kMaxZoom = 22.0;

    Impl(uint32_t w, uint32_t h, float pixel_ratio)
        : run_loop()
        , frontend(mbgl::Size{w, h},
                   pixel_ratio,
                   // Continuous mode requires Flush so the framebuffer is
                   // presented after each renderOnce() call.
                   mbgl::gfx::HeadlessBackend::SwapBehaviour::Flush)
        , map(frontend,
              mbgl::MapObserver::nullObserver(),
              mbgl::MapOptions()
                  .withMapMode(mbgl::MapMode::Continuous)
                  .withSize(mbgl::Size{w, h})
                  .withPixelRatio(pixel_ratio),
              mbgl::ResourceOptions::Default(),
              mbgl::ClientOptions())
    {}

    void tick_animation() {
        if (!anim.active) return;
        using Clock = std::chrono::steady_clock;

        const auto now = Clock::now();
        const double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                   now - anim.start_time).count();
        double t = std::clamp(elapsed / static_cast<double>(anim.duration_ms), 0.0, 1.0);
        double k = maplibre_godot::ease_in_out(t);

        // Center -- hold at start for the first center_hold_ratio, then sweep
        double k_center;
        if (t <= anim.center_hold_ratio) {
            double t_hold = anim.center_hold_ratio > 0.0
                            ? t / anim.center_hold_ratio : 1.0;
            k_center = 0.10 * maplibre_godot::ease_in_out(t_hold);
        } else {
            double t_rest = (t - anim.center_hold_ratio) /
                            std::max(1e-6, 1.0 - anim.center_hold_ratio);
            k_center = 0.10 + 0.90 * maplibre_godot::ease_in_out(t_rest);
        }
        mbgl::LatLng c{
            maplibre_godot::lerp(anim.start_center.latitude(),  anim.target_center.latitude(),  k_center),
            maplibre_godot::lerp(anim.start_center.longitude(), anim.target_center.longitude(), k_center)};

        // Zoom -- two-phase: zoom-out then zoom-in
        double z;
        if (t <= anim.mid_ratio) {
            double t0 = anim.mid_ratio > 0.0 ? t / anim.mid_ratio : 1.0;
            z = maplibre_godot::lerp(anim.start_zoom, anim.mid_zoom, maplibre_godot::ease_in_out(t0));
        } else {
            double t1 = (t - anim.mid_ratio) /
                        std::max(1e-6, 1.0 - anim.mid_ratio);
            z = maplibre_godot::lerp(anim.mid_zoom, anim.target_zoom, maplibre_godot::ease_in_out(t1));
        }

        mbgl::CameraOptions cam;
        cam.center = c;
        cam.zoom   = z;
        map.jumpTo(cam);

        if (t >= 1.0) anim.active = false;
    }
};

// ---------------------------------------------------------------------------
// MapRuntime
// ---------------------------------------------------------------------------

MapRuntime::MapRuntime(uint32_t w, uint32_t h, float pixel_ratio)
    : impl_(std::make_unique<Impl>(w, h, pixel_ratio))
{}

MapRuntime::~MapRuntime() = default;

RenderResult MapRuntime::tick() {
    RenderResult result;
    try {
        // 1. Pump the event loop (tile loads, network callbacks, etc.)
        impl_->run_loop.runOnce();

        // 2. Advance the fly_to animation (updates camera via jumpTo)
        impl_->tick_animation();

        // 3. Render one frame and read the pixels
        if (auto* backend = impl_->frontend.getBackend()) {
            mbgl::gfx::BackendScope scope{*backend};
            impl_->frontend.renderOnce(impl_->map);
            mbgl::PremultipliedImage pre = impl_->frontend.readStillImage();

            if (!pre.valid()) {
                result.error   = "readStillImage returned invalid image";
                result.success = false;
                return result;
            }

            // Convert premultiplied → straight alpha for correct Godot display
            mbgl::UnassociatedImage img = mbgl::util::unpremultiply(std::move(pre));

            result.width   = img.size.width;
            result.height  = img.size.height;
            result.pixels.resize(img.bytes());
            std::memcpy(result.pixels.data(), img.data.get(), img.bytes());
            result.updated = true;
            result.success = true;
        } else {
            result.error   = "getBackend() returned null";
            result.success = false;
        }
    } catch (const std::exception& e) {
        result.error   = e.what();
        result.success = false;
    } catch (...) {
        result.error   = "unknown error in tick()";
        result.success = false;
    }
    return result;
}

void MapRuntime::set_style_url(const std::string& url) {
    if (url != impl_->last_style_url) {
        impl_->map.getStyle().loadURL(url);
        impl_->last_style_url = url;
    }
}

void MapRuntime::jump_to(double lat, double lon, double zoom,
                         double bearing, double pitch) {
    impl_->anim.active = false; // cancel any in-progress animation
    mbgl::CameraOptions cam;
    cam.center  = mbgl::LatLng{lat, lon};
    cam.zoom    = zoom;
    cam.bearing = bearing;
    cam.pitch   = pitch;
    impl_->map.jumpTo(cam);
}

void MapRuntime::set_pitch(double pitch) {
    const auto cam = impl_->map.getCameraOptions();
    mbgl::CameraOptions next;
    next.center  = cam.center.value_or(mbgl::LatLng{0, 0});
    next.zoom    = cam.zoom.value_or(1.0);
    next.bearing = cam.bearing.value_or(0.0);
    next.pitch   = pitch;
    impl_->map.jumpTo(next);
}

void MapRuntime::set_bearing(double bearing) {
    const auto cam = impl_->map.getCameraOptions();
    mbgl::CameraOptions next;
    next.center  = cam.center.value_or(mbgl::LatLng{0, 0});
    next.zoom    = cam.zoom.value_or(1.0);
    next.pitch   = cam.pitch.value_or(0.0);
    next.bearing = bearing;
    impl_->map.jumpTo(next);
}

void MapRuntime::fly_to(double lat, double lon, double zoom, double current_zoom_hint) {
    const auto cam      = impl_->map.getCameraOptions();
    const mbgl::LatLng target{lat, lon};
    const mbgl::LatLng start = cam.center.value_or(target);
    // Prefer the camera's own zoom; fall back to the caller's tracked zoom
    // because on Windows wgpu-Vulkan getCameraOptions().zoom returns nullopt.
    const double start_zoom  = cam.zoom.value_or(current_zoom_hint);

    double dist = approx_distance_deg(start.latitude(), start.longitude(), lat, lon);
    double mid_zoom = fly_to_mid_zoom(start_zoom, dist, Impl::kMinZoom, Impl::kMaxZoom);

    auto& a            = impl_->anim;
    a.active           = true;
    a.start_center     = start;
    a.target_center    = target;
    a.start_zoom       = start_zoom;
    a.target_zoom      = zoom;
    a.mid_zoom         = mid_zoom;
    a.mid_ratio        = 0.60;
    a.center_hold_ratio = 0.20;
    a.start_time       = std::chrono::steady_clock::now();
    a.duration_ms      = 2500;
}

void MapRuntime::resize(uint32_t w, uint32_t h) {
    impl_->frontend.setSize(mbgl::Size{w, h});
    impl_->map.setSize(mbgl::Size{w, h});
}

ScreenPoint MapRuntime::geo_to_screen(double lat, double lon) const {
    const auto point = impl_->map.pixelForLatLng(mbgl::LatLng{lat, lon});
    return ScreenPoint{point.x, point.y};
}

GeoPoint MapRuntime::screen_to_geo(double x, double y) const {
    const auto lat_lng = impl_->map.latLngForPixel(mbgl::ScreenCoordinate{x, y});
    return GeoPoint{lat_lng.latitude(), lat_lng.longitude()};
}

} // namespace maplibre_godot
