#pragma once

#include <mini/ini.h>
#include <string>

namespace suite::persistent
{
    namespace intern
    {
        extern mINI::INIFile file;
        extern mINI::INIStructure data;
    }

    mINI::INIStructure& settings();

    void save_settings();
}

