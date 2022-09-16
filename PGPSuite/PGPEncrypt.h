#pragma once

#include "pgpsuite_common.h"
#include "rnp_wrappers.h"
#include "IOTools.h"

namespace pgp
{
    /* @brief encrypt bytes from data start till data + size
    @param data: Start of bytes to be encrypted 
    @param size: data + size , is end of bytes to be encrypted
    @param pubkey_file: the filename of the recipient's public key @TODO(allow adding multiple keys)
    @param save_to: preferred filename to save encrypted data to 
    @param add_password: wether to add a password to the encrypted text 
    @return boolean indicating success or failure of encryption */
    bool encrypt_text(uint8_t* data, size_t size, std::string pubkey_file, std::string save_to = "message.asc", bool add_password = false);
}
