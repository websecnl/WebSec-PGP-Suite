#include "PersistentData.h"

using namespace suite::persistent;

mINI::INIFile suite::persistent::intern::file = mINI::INIFile("settings.ini");
mINI::INIStructure suite::persistent::intern::data{};

static void init_settings_file()
{
    intern::file.read(intern::data);
}

mINI::INIStructure& suite::persistent::settings()
{
    static bool initialized{ false };
    if (!initialized)
    {
        init_settings_file();
        initialized = true;
    }
    
    return(intern::data);
}

void suite::persistent::save_settings()
{
    intern::file.write(intern::data);
}
