#include "map_view_node.hpp"

#include "map_runtime.hpp"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <chrono>

namespace godot {

// ---------------------------------------------------------------------------
// Constructor / destructor
// ---------------------------------------------------------------------------

MapLibreMap::MapLibreMap() = default;

// Defined here so the compiler sees the complete MapRuntime type when
// std::unique_ptr<MapRuntime> is destroyed.
MapLibreMap::~MapLibreMap() = default;

// ---------------------------------------------------------------------------
// _bind_methods
// ---------------------------------------------------------------------------

void MapLibreMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_style_url", "style_url"), &MapLibreMap::set_style_url);
    ClassDB::bind_method(D_METHOD("get_style_url"),              &MapLibreMap::get_style_url);
    ClassDB::bind_method(D_METHOD("fly_to", "lat", "lon", "zoom"), &MapLibreMap::fly_to);
    ClassDB::bind_method(D_METHOD("jump_to", "lat", "lon", "zoom", "bearing", "pitch"),
                         &MapLibreMap::jump_to);
    ClassDB::bind_method(D_METHOD("set_pitch",   "pitch"),   &MapLibreMap::set_pitch);
    ClassDB::bind_method(D_METHOD("set_bearing", "bearing"), &MapLibreMap::set_bearing);
    ClassDB::bind_method(D_METHOD("get_current_lat"),     &MapLibreMap::get_current_lat);
    ClassDB::bind_method(D_METHOD("get_current_lon"),     &MapLibreMap::get_current_lon);
    ClassDB::bind_method(D_METHOD("get_current_zoom"),    &MapLibreMap::get_current_zoom);
    ClassDB::bind_method(D_METHOD("get_current_bearing"), &MapLibreMap::get_current_bearing);
    ClassDB::bind_method(D_METHOD("get_current_pitch"),   &MapLibreMap::get_current_pitch);
    ClassDB::bind_method(D_METHOD("get_last_render_ms"),      &MapLibreMap::get_last_render_ms);
    ClassDB::bind_method(D_METHOD("get_runtime_description"), &MapLibreMap::get_runtime_description);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "style_url"), "set_style_url", "get_style_url");
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void MapLibreMap::set_style_url(const String& p_style_url) {
    style_url = p_style_url;
    if (runtime_)
        runtime_->set_style_url(std::string(p_style_url.utf8().get_data()));
}

String MapLibreMap::get_style_url() const {
    return style_url;
}

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------

void MapLibreMap::fly_to(double p_lat, double p_lon, double p_zoom) {
    current_lat  = p_lat;
    current_lon  = p_lon;
    current_zoom = p_zoom;
    if (runtime_)
        runtime_->fly_to(p_lat, p_lon, p_zoom);
}

void MapLibreMap::jump_to(double p_lat, double p_lon, double p_zoom,
                           double p_bearing, double p_pitch) {
    current_lat     = p_lat;
    current_lon     = p_lon;
    current_zoom    = p_zoom;
    current_bearing = p_bearing;
    current_pitch   = p_pitch;
    if (runtime_)
        runtime_->jump_to(p_lat, p_lon, p_zoom, p_bearing, p_pitch);
}

void MapLibreMap::set_pitch(double p_pitch) {
    current_pitch = p_pitch;
    if (runtime_)
        runtime_->set_pitch(p_pitch);
}

void MapLibreMap::set_bearing(double p_bearing) {
    current_bearing = p_bearing;
    if (runtime_)
        runtime_->set_bearing(p_bearing);
}

double MapLibreMap::get_current_lat()     const { return current_lat; }
double MapLibreMap::get_current_lon()     const { return current_lon; }
double MapLibreMap::get_current_zoom()    const { return current_zoom; }
double MapLibreMap::get_current_bearing() const { return current_bearing; }
double MapLibreMap::get_current_pitch()   const { return current_pitch; }

// ---------------------------------------------------------------------------
// get_last_render_ms / get_runtime_description
// ---------------------------------------------------------------------------

int64_t MapLibreMap::get_last_render_ms() const {
    return last_render_ms_;
}

String MapLibreMap::get_runtime_description() const {
    String desc = "MapLibreMap: style=";
    desc += style_url;
    desc += " lat="     + String::num(current_lat,     4);
    desc += " lon="     + String::num(current_lon,     4);
    desc += " zoom="    + String::num(current_zoom,    2);
    desc += " bearing=" + String::num(current_bearing, 1);
    desc += " pitch="   + String::num(current_pitch,   1);
    desc += runtime_ ? " [renderer:active]" : " [renderer:not-initialized]";
    return desc;
}

// ---------------------------------------------------------------------------
// _notification — READY starts the renderer; PROCESS drives it every frame
// ---------------------------------------------------------------------------

void MapLibreMap::_notification(int p_what) {
    if (p_what == Node::NOTIFICATION_READY) {
        UtilityFunctions::print("MapLibreMap: initializing headless renderer (Continuous mode)");
        const Vector2 sz = get_size();
        render_width_  = sz.x > 0 ? static_cast<uint32_t>(sz.x) : 512;
        render_height_ = sz.y > 0 ? static_cast<uint32_t>(sz.y) : 512;
        runtime_ = std::make_unique<maplibre_godot::MapRuntime>(render_width_, render_height_, 1.0f);
        runtime_->set_style_url(std::string(style_url.utf8().get_data()));
        runtime_->jump_to(current_lat, current_lon, current_zoom,
                          current_bearing, current_pitch);
        set_process(true);
        UtilityFunctions::print("MapLibreMap: ready - _process loop started");
        return;
    }

    if (p_what == Node::NOTIFICATION_PROCESS) {
        // Resize the renderer if the node's display size has changed.
        const Vector2 sz = get_size();
        const uint32_t w = sz.x > 0 ? static_cast<uint32_t>(sz.x) : render_width_;
        const uint32_t h = sz.y > 0 ? static_cast<uint32_t>(sz.y) : render_height_;
        if (w != render_width_ || h != render_height_) {
            render_width_  = w;
            render_height_ = h;
            runtime_->resize(w, h);
        }

        using Clock = std::chrono::steady_clock;
        const auto t0 = Clock::now();

        const auto result = runtime_->tick();

        const auto tick_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - t0).count();

        if (!result.success) {
            UtilityFunctions::push_error(
                String("MapLibreMap: tick failed — ") + String(result.error.c_str()));
            return;
        }

        if (!result.updated) return;

        last_render_ms_ = tick_ms;

        // Copy raw RGBA bytes into a Godot PackedByteArray.
        PackedByteArray pixel_data;
        pixel_data.resize(static_cast<int64_t>(result.pixels.size()));
        memcpy(pixel_data.ptrw(), result.pixels.data(), result.pixels.size());

        // Create / update the Image.
        Ref<Image> image = Image::create_from_data(
            static_cast<int32_t>(result.width),
            static_cast<int32_t>(result.height),
            false,               // no mipmaps
            Image::FORMAT_RGBA8,
            pixel_data);

        if (image.is_null()) {
            UtilityFunctions::push_error("MapLibreMap: failed to create Image from tick result");
            return;
        }

        // Reuse the existing ImageTexture to avoid per-frame reallocation.
        // If dimensions changed (after a resize), create a new texture.
        Ref<ImageTexture> tex = get_texture();
        if (tex.is_valid() &&
            tex->get_width()  == static_cast<int32_t>(result.width) &&
            tex->get_height() == static_cast<int32_t>(result.height)) {
            tex->update(image);
        } else {
            set_texture(ImageTexture::create_from_image(image));
        }
    }
}

} // namespace godot
