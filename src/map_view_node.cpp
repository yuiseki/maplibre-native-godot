#include "map_view_node.hpp"

#include "map_runtime.hpp"

#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

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
    ClassDB::bind_method(D_METHOD("set_pitch",   "pitch"),   &MapLibreMap::set_pitch);
    ClassDB::bind_method(D_METHOD("set_bearing", "bearing"), &MapLibreMap::set_bearing);
    ClassDB::bind_method(D_METHOD("get_current_lat"),     &MapLibreMap::get_current_lat);
    ClassDB::bind_method(D_METHOD("get_current_lon"),     &MapLibreMap::get_current_lon);
    ClassDB::bind_method(D_METHOD("get_current_zoom"),    &MapLibreMap::get_current_zoom);
    ClassDB::bind_method(D_METHOD("get_current_bearing"), &MapLibreMap::get_current_bearing);
    ClassDB::bind_method(D_METHOD("get_current_pitch"),   &MapLibreMap::get_current_pitch);
    ClassDB::bind_method(D_METHOD("render_once"),             &MapLibreMap::render_once);
    ClassDB::bind_method(D_METHOD("get_runtime_description"), &MapLibreMap::get_runtime_description);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "style_url"), "set_style_url", "get_style_url");
}

// ---------------------------------------------------------------------------
// Properties
// ---------------------------------------------------------------------------

void MapLibreMap::set_style_url(const String& p_style_url) {
    style_url = p_style_url;
    render_once();
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
    render_once();
}

void MapLibreMap::set_pitch(double p_pitch) {
    current_pitch = p_pitch;
    render_once();
}

void MapLibreMap::set_bearing(double p_bearing) {
    current_bearing = p_bearing;
    render_once();
}

double MapLibreMap::get_current_lat()     const { return current_lat; }
double MapLibreMap::get_current_lon()     const { return current_lon; }
double MapLibreMap::get_current_zoom()    const { return current_zoom; }
double MapLibreMap::get_current_bearing() const { return current_bearing; }
double MapLibreMap::get_current_pitch()   const { return current_pitch; }

// ---------------------------------------------------------------------------
// render_once — calls MapRuntime and feeds real pixels into ImageTexture
// ---------------------------------------------------------------------------

void MapLibreMap::render_once() {
    // Lazy-initialize the persistent headless renderer on first call.
    // 512×512 is a reasonable default render resolution for a PoC.
    if (!runtime_) {
        UtilityFunctions::print("MapLibreMap: initializing headless renderer");
        runtime_ = std::make_unique<maplibre_godot::MapRuntime>(512, 512, 1.0f);
    }

    const auto result = runtime_->render(
        std::string(style_url.utf8().get_data()),
        current_lat, current_lon, current_zoom,
        current_bearing, current_pitch);

    if (!result.success) {
        UtilityFunctions::push_error(
            String("MapLibreMap: render failed — ") + String(result.error.c_str()));
        return;
    }

    // Copy the raw RGBA bytes into a Godot PackedByteArray.
    PackedByteArray pixel_data;
    pixel_data.resize(static_cast<int64_t>(result.pixels.size()));
    memcpy(pixel_data.ptrw(), result.pixels.data(), result.pixels.size());

    // Create a Godot Image from the pixel data.
    // maplibre-native produces premultiplied RGBA; for fully-opaque map tiles
    // premultiplied and straight-alpha are identical, so no conversion is needed.
    Ref<Image> image = Image::create_from_data(
        static_cast<int32_t>(result.width),
        static_cast<int32_t>(result.height),
        false,               // no mipmaps
        Image::FORMAT_RGBA8,
        pixel_data);

    if (image.is_null()) {
        UtilityFunctions::push_error("MapLibreMap: failed to create Image from render result");
        return;
    }

    Ref<ImageTexture> texture = ImageTexture::create_from_image(image);
    if (texture.is_null()) {
        UtilityFunctions::push_error("MapLibreMap: failed to create ImageTexture");
        return;
    }
    set_texture(texture);
}

// ---------------------------------------------------------------------------
// get_runtime_description
// ---------------------------------------------------------------------------

String MapLibreMap::get_runtime_description() const {
    String desc = "MapLibreMap: style=";
    desc += style_url;
    desc += " lat=" + String::num(current_lat, 4);
    desc += " lon=" + String::num(current_lon, 4);
    desc += " zoom=" + String::num(current_zoom, 2);
    desc += " bearing=" + String::num(current_bearing, 1);
    desc += " pitch=" + String::num(current_pitch, 1);
    desc += runtime_ ? " [renderer:active]" : " [renderer:not-initialized]";
    return desc;
}

// ---------------------------------------------------------------------------
// _notification
// ---------------------------------------------------------------------------

void MapLibreMap::_notification(int p_what) {
    if (p_what == Node::NOTIFICATION_READY) {
        UtilityFunctions::print("MapLibreMap: ready — starting initial render");
        render_once();
        UtilityFunctions::print("MapLibreMap: initial render complete");
    }
}

} // namespace godot
