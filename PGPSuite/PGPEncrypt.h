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
    /* @brief encrypt bytes from data start till data + size
    @param data: Start of bytes to be encrypted 
    @param size: data + size , is end of bytes to be encrypted
    @param pubkey_file: the filename of the recipient's public key @TODO(allow adding multiple keys)
    @param userid: the userid of the key
    @param save_to: preferred filename to save encrypted data to 
    @param password: password to encrypt text with, no password if left empty
    @return boolean indicating success or failure of encryption */
    OpRes encrypt_text(uint8_t* data, size_t size, std::string pubkey_file, std::string userid, std::string save_to = "message.asc", std::string password = {});
}
