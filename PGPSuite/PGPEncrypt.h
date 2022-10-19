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
