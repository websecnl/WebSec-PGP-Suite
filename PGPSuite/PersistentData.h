/*
 *
 * Copyright (c) 2018-2023
 * Author: WebSec B.V.
 * Developer: Koen Blok
 * Website: https://websec.nl
 *
 * Permission to use, copy, modify, distribute this software
 * and its documentation for non-commercial purposes is hereby granted exclusivley
 * under the terms of the GNU GPLv3 License.
 *
 * Most importantly:
 *  1. The above copyright notice appear in all copies and supporting documents.
 *  2. The application / code will not be used or reused for commercial purposes.
 *  3. All modifications are documented.
 *  4. All new releases will remain open source and contain the same license.
 *
 * WebSec B.V. makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * please read the full license agreement for more information:
 * https://github.com/websecnl/PGPSuite/LICENSE.md
 */
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

