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

#include "pgpsuite_common.h"
#include "rnp_wrappers.h"
#include "IOTools.h"
#include "Utils.h"

namespace pgp
{
    /* should go somewhere else, but alas, he lives here now */
    bool cin_pass_provider(rnp_ffi_t           ffi,
        void* app_ctx,
        rnp_key_handle_t    key,
        const char* pgp_context,
        char                buf[],
        size_t              buf_len);

    /* @brief Decrypt files using secret key 
    @param secring_file: Filename of secret keyring
    @param encrypted_file: Filename with encrypted file
    @param output_fname: Filename of the decrypted data,
    if empty, name will be same as encrypted file minus .asc
    @param passprovider: function pointer to a password provider */
    OpRes decrypt_text(
        std::string encrypted_file = "message.asc",
        std::string output_fname = "",
        rnp_password_cb passprovider = cin_pass_provider, void* context = nullptr,
        std::string secring_file = {});
}
