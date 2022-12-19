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

namespace suite
{
    enum
    {
        ID_Hello = wxID_HIGHEST + 1,

        /* id's to link button to input field */
        ID_OPEN_PUBKEY,
        ID_OPEN_SECKEY,
        ID_OPEN_ENC_FILE,
        ID_OPEN_FILE,
        ID_EDIT_TEXT,

        /* actions */
        ID_GENERATE_KEY,
        ID_ENCRYPT_FILE,
        ID_DECRYPT_FILE,
        ID_ENC_TYPE_RADIO_CHANGED,
        ID_SHOW_GENERATE_SETTINGS,
        ID_CHECK_VERSION,
        ID_STARTUP_CHECKBOX_SETTING,
        ID_KEYID_SELECTION,

        /* menu's */
        ID_REGISTER_EXTENSION,
        ID_UNREGISTER_EXTENSION,

        /* unused */
        ID_SAVE_FILE,
    };
}