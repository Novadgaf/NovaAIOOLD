#include "../plugin_sdk/plugin_sdk.hpp"

// Declare plugin name & supported champions
//
PLUGIN_NAME("Nova");
SUPPORTED_CHAMPIONS(champion_id::Garen, champion_id::Evelynn);


#include "garen.h"
#include "evelynn.h"

// Entry point of plugin
//
PLUGIN_API bool on_sdk_load(plugin_sdk_core* plugin_sdk_good)
{
    // Global declaretion macro
    //
    DECLARE_GLOBALS(plugin_sdk_good);

    //Switch by myhero champion id
    //
    switch (myhero->get_champion())
    {
    case champion_id::Garen:
        garen::load();
        break;
    case champion_id::Evelynn:
        evelynn::load();
        break;
    default:
        // We don't support this champ, print message and return false (core will not load this plugin and on_sdk_unload will be never called)
        //
        console->print("Champion %s is not supported!", myhero->get_model_cstr());
        return false;
    }

    // Return success, our plugin will be loaded now
    //
    return true;
}

// Unload function, only when on_sdk_load returned true
//
PLUGIN_API void on_sdk_unload()
{
    switch (myhero->get_champion())
    {
    case champion_id::Garen:
        // Unload garen script
        //
        garen::unload();
        break;
    default:
        break;
    }
}