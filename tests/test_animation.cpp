#include "../src/animation.hpp"

#include <cassert>
#include <cmath>
#include <cstdio>

using namespace maplibre_godot;

static constexpr double kEps = 1e-9;

static void near(double actual, double expected, const char* label) {
    if (std::abs(actual - expected) > kEps) {
        std::fprintf(stderr, "FAIL: %s: expected %.12f, got %.12f\n",
                     label, expected, actual);
        std::abort();
    }
}

// ---------------------------------------------------------------------------
// ease_in_out
// ---------------------------------------------------------------------------

static void test_ease_in_out_boundaries() {
    near(ease_in_out(0.0), 0.0, "ease(0)");
    near(ease_in_out(1.0), 1.0, "ease(1)");
    near(ease_in_out(0.5), 0.5, "ease(0.5) midpoint");
}

static void test_ease_in_out_symmetry() {
    // ease(t) + ease(1-t) should equal 1 (point symmetry around (0.5, 0.5))
    for (double t = 0.0; t <= 1.0; t += 0.05) {
        double sum = ease_in_out(t) + ease_in_out(1.0 - t);
        near(sum, 1.0, "ease symmetry");
    }
}

static void test_ease_in_out_monotonic() {
    double prev = ease_in_out(0.0);
    for (double t = 0.01; t <= 1.0; t += 0.01) {
        double val = ease_in_out(t);
        assert(val >= prev && "ease_in_out must be monotonically increasing");
        prev = val;
    }
}

// ---------------------------------------------------------------------------
// lerp
// ---------------------------------------------------------------------------

static void test_lerp() {
    near(lerp(0.0, 10.0, 0.0), 0.0,  "lerp(0)");
    near(lerp(0.0, 10.0, 1.0), 10.0, "lerp(1)");
    near(lerp(0.0, 10.0, 0.5), 5.0,  "lerp(0.5)");
    near(lerp(-5.0, 5.0, 0.5), 0.0,  "lerp negative");
}

// ---------------------------------------------------------------------------
// approx_distance_deg
// ---------------------------------------------------------------------------

static void test_distance_same_point() {
    double d = approx_distance_deg(35.6762, 139.6503, 35.6762, 139.6503);
    near(d, 0.0, "distance same point");
}

static void test_distance_known_cities() {
    // Tokyo to Paris: ~86 degrees great-circle (rough approximation)
    double d = approx_distance_deg(35.6762, 139.6503, 48.8566, 2.3522);
    assert(d > 50.0 && d < 120.0 && "Tokyo-Paris distance in degrees");

    // Tokyo to New York (~168 degrees via this approximation)
    double d2 = approx_distance_deg(35.6762, 139.6503, 40.7128, -74.0060);
    assert(d2 > 100.0 && d2 < 200.0 && "Tokyo-NYC distance in degrees");
}

static void test_distance_symmetry() {
    double d1 = approx_distance_deg(35.6762, 139.6503, 48.8566, 2.3522);
    double d2 = approx_distance_deg(48.8566, 2.3522, 35.6762, 139.6503);
    near(d1, d2, "distance symmetry");
}

// ---------------------------------------------------------------------------
// fly_to_mid_zoom
// ---------------------------------------------------------------------------

static void test_mid_zoom_short_distance() {
    // Short distance: zoom out by at least 2 (the minimum delta)
    double mid = fly_to_mid_zoom(15.0, 0.1);
    assert(mid <= 13.0 && "short distance mid_zoom");
    assert(mid >= 0.0 && "mid_zoom >= min");
}

static void test_mid_zoom_long_distance() {
    // Long distance: zoom out more aggressively
    double mid_short = fly_to_mid_zoom(15.0, 1.0);
    double mid_long  = fly_to_mid_zoom(15.0, 50.0);
    assert(mid_long <= mid_short && "longer distance zooms out more");
}

static void test_mid_zoom_clamped() {
    // Starting at low zoom with large distance should clamp to min_zoom
    double mid = fly_to_mid_zoom(5.0, 100.0, 0.0, 22.0);
    assert(mid >= 0.0 && "mid_zoom clamped to min");
    assert(mid <= 22.0 && "mid_zoom clamped to max");
}

// ---------------------------------------------------------------------------

int main() {
    test_ease_in_out_boundaries();
    test_ease_in_out_symmetry();
    test_ease_in_out_monotonic();
    test_lerp();
    test_distance_same_point();
    test_distance_known_cities();
    test_distance_symmetry();
    test_mid_zoom_short_distance();
    test_mid_zoom_long_distance();
    test_mid_zoom_clamped();

    std::printf("All animation tests passed.\n");
    return 0;
}
