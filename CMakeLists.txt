cmake_minimum_required(VERSION 3.24)

project(maplibre-native-godot-poc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GODOT_CPP_DIR "${CMAKE_SOURCE_DIR}/third_party/godot-cpp" CACHE PATH "Path to a godot-cpp checkout")
set(MLN_SOURCE_DIR "/home/yuiseki/Workspaces/repos/_yuiseki/_fork/maplibre-native" CACHE PATH "Path to maplibre-native source")
set(MLN_BUILD_DIR "${CMAKE_SOURCE_DIR}/build/maplibre-native-linux-clean" CACHE PATH "Path to maplibre-native build output")

# We use individual PIC-compiled libs rather than the amalgam to avoid bundled
# system static libs (libssl, libuv…) that were not built with -fPIC.
set(MLN_CORE_LIB "${MLN_BUILD_DIR}/libmbgl-core.a" CACHE FILEPATH "Path to libmbgl-core.a")

if(NOT EXISTS "${GODOT_CPP_DIR}/CMakeLists.txt")
  message(FATAL_ERROR "godot-cpp was not found at ${GODOT_CPP_DIR}. Check out godot-cpp there before building the extension.")
endif()

if(NOT EXISTS "${MLN_CORE_LIB}")
  message(FATAL_ERROR "maplibre-native core library was not found at ${MLN_CORE_LIB}. Run scripts/build_maplibre_native_linux.sh first.")
endif()

add_subdirectory("${GODOT_CPP_DIR}" "${CMAKE_BINARY_DIR}/godot-cpp")

add_library(maplibre_native_godot SHARED
  src/register_types.cpp
  src/map_view_node.cpp
  src/map_runtime.cpp
)

target_include_directories(maplibre_native_godot PRIVATE
  "${CMAKE_SOURCE_DIR}/src"
  "${MLN_SOURCE_DIR}/include"
  "${MLN_SOURCE_DIR}/platform/default/include"
  # Vendor headers transitively required by the public maplibre-native headers
  "${MLN_SOURCE_DIR}/vendor/maplibre-native-base/include"
  "${MLN_SOURCE_DIR}/vendor/maplibre-native-base/deps/variant/include"
  "${MLN_SOURCE_DIR}/vendor/maplibre-native-base/deps/geometry.hpp/include"
  "${MLN_SOURCE_DIR}/vendor/maplibre-native-base/deps/geojson.hpp/include"
  "${MLN_SOURCE_DIR}/vendor/maplibre-native-base/deps/geojson-vt-cpp/include"
  "${MLN_SOURCE_DIR}/vendor/boost/include"
  "${MLN_SOURCE_DIR}/vendor/rapidjson/include"
  "${MLN_SOURCE_DIR}/vendor/unordered_dense/include"
  "${MLN_SOURCE_DIR}/vendor/eternal/include"
  "${MLN_SOURCE_DIR}/vendor/protozero/include"
  "${MLN_SOURCE_DIR}/vendor/webgpu-cpp"
  "${MLN_SOURCE_DIR}/vendor/webgpu-cpp/wgpu-native"
  "${MLN_SOURCE_DIR}/vendor/wgpu-native/ffi"
)

# Maplibre-native static libs (all compiled with -fPIC via our build script).
# Use --whole-archive so that self-registering factory objects are not dropped.
set(MLN_VENDOR_LIBS
  "${MLN_BUILD_DIR}/libmbgl-core.a"
  "${MLN_BUILD_DIR}/libmbgl-freetype.a"
  "${MLN_BUILD_DIR}/libmbgl-harfbuzz.a"
  "${MLN_BUILD_DIR}/libmbgl-vendor-csscolorparser.a"
  "${MLN_BUILD_DIR}/libmbgl-vendor-nunicode.a"
  "${MLN_BUILD_DIR}/libmbgl-vendor-sqlite.a"
  "${MLN_BUILD_DIR}/libmbgl-vendor-parsedate.a"
  "${MLN_BUILD_DIR}/vendor/maplibre-tile-spec/cpp/libmlt-cpp.a"
  "${MLN_SOURCE_DIR}/vendor/wgpu-native/target/x86_64-unknown-linux-gnu/release/libwgpu_native.a"
)

target_link_libraries(maplibre_native_godot PRIVATE
  godot-cpp
  -Wl,--whole-archive
  ${MLN_VENDOR_LIBS}
  -Wl,--no-whole-archive
)

if(UNIX AND NOT APPLE)
  target_link_libraries(maplibre_native_godot PRIVATE
    # System libs linked dynamically — these were not built with -fPIC by us
    dl
    m
    pthread
    ssl
    crypto
    curl
    uv
    png
    jpeg
    webp
    bz2
    z
    icuuc
    icudata
    icui18n
    # GPU / display
    GL
    X11
    # Vulkan (required by wgpu-native)
    vulkan
  )
endif()

set(MLNG_OUTPUT_NAME "maplibre_native_godot.linux.template_debug.x86_64")
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  set(MLNG_OUTPUT_NAME "maplibre_native_godot.linux.template_release.x86_64")
endif()

set_target_properties(maplibre_native_godot PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
  PREFIX "lib"
  OUTPUT_NAME "${MLNG_OUTPUT_NAME}"
)
