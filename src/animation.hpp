#pragma once

#include <algorithm>
#include <cmath>

namespace maplibre_godot {

/// Cubic ease-in-out: smooth acceleration then deceleration.
/// Input t in [0, 1], output in [0, 1].
inline double ease_in_out(double t) {
    return t < 0.5 ? 4.0 * t * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;
}

/// Linear interpolation between a and b by factor f.
inline double lerp(double a, double b, double f) {
    return a + (b - a) * f;
}

/// Compute the mid-zoom level for a fly_to animation arc.
/// Zooms out further for longer distances.
inline double fly_to_mid_zoom(double start_zoom, double distance_deg,
                              double min_zoom = 0.0, double max_zoom = 22.0) {
    double zoom_out_delta = std::max(2.0, 8.0 + std::min(3.0, distance_deg / 8.0));
    return std::clamp(start_zoom - zoom_out_delta, min_zoom, max_zoom);
}

/// Approximate great-circle distance in degrees between two lat/lon points.
inline double approx_distance_deg(double lat1, double lon1,
                                   double lat2, double lon2) {
    auto deg2rad = [](double d) { return d * M_PI / 180.0; };
    double lat1r = deg2rad(lat1);
    double lat2r = deg2rad(lat2);
    double x = deg2rad(lon2 - lon1) * std::cos((lat1r + lat2r) * 0.5);
    double y = lat2r - lat1r;
    return std::sqrt(x * x + y * y) * 180.0 / M_PI;
}

} // namespace maplibre_godot
