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
    OpRes decrypt_text(std::string encrypted_file = "message.asc", std::string output_fname = "", rnp_password_cb passprovider = cin_pass_provider, std::string secring_file = {});
}
