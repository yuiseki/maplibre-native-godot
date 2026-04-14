#include "map_view_node.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void initialize_maplibre_native_godot(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    UtilityFunctions::print("maplibre_native_godot: initialize");
    ClassDB::register_class<MapLibreMap>();
}

void uninitialize_maplibre_native_godot(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    UtilityFunctions::print("maplibre_native_godot: uninitialize");
}

extern "C" {
GDExtensionBool GDE_EXPORT maplibre_native_godot_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    const GDExtensionClassLibraryPtr p_library,
    GDExtensionInitialization* r_initialization) {
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_maplibre_native_godot);
    init_obj.register_terminator(uninitialize_maplibre_native_godot);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}
