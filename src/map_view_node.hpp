#pragma once

#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/vector2.hpp>

#include <memory>

// Forward-declare so the header stays free of maplibre-native includes.
namespace maplibre_godot {
class MapRuntime;
} // namespace maplibre_godot

namespace godot {

class MapLibreMap : public TextureRect {
    GDCLASS(MapLibreMap, TextureRect)

public:
    MapLibreMap();
    ~MapLibreMap() override; // defined in .cpp where MapRuntime is complete

    void set_style_url(const String& p_style_url);
    String get_style_url() const;

    void fly_to(double p_lat, double p_lon, double p_zoom);
    void jump_to(double p_lat, double p_lon, double p_zoom,
                 double p_bearing = 0.0, double p_pitch = 0.0);
    void set_pitch(double p_pitch);
    void set_bearing(double p_bearing);
    Vector2 geo_to_screen(double p_lat, double p_lon) const;
    Dictionary screen_to_geo(double p_x, double p_y) const;

    double get_current_lat() const;
    double get_current_lon() const;
    double get_current_zoom() const;
    double get_current_bearing() const;
    double get_current_pitch() const;

    int64_t get_last_render_ms() const;   // ms taken by the most recent tick()
    String get_runtime_description() const;
    void _notification(int p_what);

protected:
    static void _bind_methods();

private:
    String style_url = "https://demotiles.maplibre.org/style.json";
    double current_lat     = 0.0;
    double current_lon     = 0.0;
    double current_zoom    = 1.0;
    double current_bearing = 0.0;
    double current_pitch   = 0.0;

    // Persistent headless renderer — initialized in NOTIFICATION_READY.
    std::unique_ptr<maplibre_godot::MapRuntime> runtime_;
    int64_t last_render_ms_ = 0;
    uint32_t render_width_  = 0;
    uint32_t render_height_ = 0;
};

} // namespace godot
