#include "map_view_node.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <cstdio>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

using namespace godot;

// ---------------------------------------------------------------------------
// Instrumentation: log every phase of DLL initialization to a file so we can
// see exactly where Error 1114 happens when loaded inside a Godot process.
// The target file path is intentionally absolute/Windows-specific because
// the CWD at DllMain time is not reliably the addon directory.
// ---------------------------------------------------------------------------

namespace {

// Log path is relative so it lands in the process CWD (usually the exe dir for
// Godot-exported games). CI can then "type" the file to dump it into logs.
const char* kDllLogFile = "mlng_init.log";

void log_phase(const char* phase) {
    FILE* f = std::fopen(kDllLogFile, "a");
    if (!f) {
        // Fall back to a platform-specific temp location if CWD is read-only.
#ifdef _WIN32
        f = std::fopen("C:\\Users\\yuiseki\\mlng_init.log", "a");
        if (!f) f = std::fopen("C:\\Windows\\Temp\\mlng_init.log", "a");
#else
        f = std::fopen("/tmp/mlng_init.log", "a");
#endif
        if (!f) return;
    }
    std::time_t now = std::time(nullptr);
    std::fprintf(f, "[%lld] %s\n", static_cast<long long>(now), phase);
    std::fclose(f);
}

struct EarlyProbe {
    EarlyProbe()  { log_phase("[static-init] EarlyProbe ctor"); }
    ~EarlyProbe() { log_phase("[static-dtor] EarlyProbe dtor"); }
};

static EarlyProbe g_early_probe;

} // namespace

#ifdef _WIN32
extern "C" BOOL WINAPI DllMain(HINSTANCE /*instance*/, DWORD reason, LPVOID /*reserved*/) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: log_phase("[DllMain] PROCESS_ATTACH"); break;
        case DLL_PROCESS_DETACH: log_phase("[DllMain] PROCESS_DETACH"); break;
        case DLL_THREAD_ATTACH:  log_phase("[DllMain] THREAD_ATTACH");  break;
        case DLL_THREAD_DETACH:  log_phase("[DllMain] THREAD_DETACH");  break;
    }
    return TRUE;
}
#endif

void initialize_maplibre_native_godot(ModuleInitializationLevel p_level) {
    log_phase("[initialize] entry");
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        log_phase("[initialize] skipped (not SCENE level)");
        return;
    }

    log_phase("[initialize] SCENE level");
    UtilityFunctions::print("maplibre_native_godot: initialize");
    ClassDB::register_class<MapLibreMap>();
    log_phase("[initialize] ClassDB::register_class done");
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
    log_phase("[library_init] entry");
    GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_maplibre_native_godot);
    init_obj.register_terminator(uninitialize_maplibre_native_godot);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    auto ret = init_obj.init();
    log_phase(ret ? "[library_init] init_obj.init OK" : "[library_init] init_obj.init FAILED");
    return ret;
}
}
