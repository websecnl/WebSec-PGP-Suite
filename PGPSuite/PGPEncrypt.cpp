#include "PGPEncrypt.h"

pgp::OpRes pgp::encrypt_text(uint8_t* data, size_t size, std::string pubkey_file, std::string userid, std::string save_to, std::string password)
{
    rnp::Input input_message;
    rnp::Output output_message;
    rnp_key_handle_t key = nullptr;

    rnp::Input input_key;
    rnp::FFI ffi("GPG", "GPG");

    if (auto res = pgp::utils::validate_strings<std::string>(pubkey_file, userid, save_to); !res) return res;

    /* Load the to be encrypted message */
    if (input_message.set_input_from_memory(data, size, false) != RNP_SUCCESS) return "Failed setting input from memory\n";

    /* Prepare the output for the encrypted message */
    if (output_message.set_output_to_path(std::forward<std::string>(save_to)) != RNP_SUCCESS) return "Failed setting output\n";

    if (!pubkey_file.empty())
    {
        /* Load key file */ /* should in the future allow for adding multiple keys */
        if (input_key.set_input_from_path(pubkey_file) != RNP_SUCCESS) return "Failed setting input\n";

        /* Attempt to read pubring.pgp for its keys */
        if (rnp_load_keys(ffi, "GPG", input_key, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
        {
            return "Failed to read: " + pubkey_file;
        }

        /* Locate key using the userid and load it into the key_handle_t */
        if (rnp_locate_key(ffi, "userid", userid.c_str(), &key) != RNP_SUCCESS)
        {
            return "Failed to locate recipient key: " + userid;
        }
    }

    rnp::EncryptOperation op(ffi, input_message, output_message);

    if (!pubkey_file.empty())
    {
        /* Recipient public key, the public keys encrypt the data so
            that the recipient can decrypt it using their secret key
            thats why we say we add the public key of the recipient */
        if (op.add_recipient(key) != RNP_SUCCESS)
        {
            return "Failed to locate recipient key: " + userid;
        }
    }

    /* Set encryption parameters */
    op.set_armor(true);
    op.set_file_name("message.txt");
    op.set_file_mtime(time(NULL));
    op.set_compression("ZIP", 6);
    op.set_cipher(RNP_ALGNAME_AES_256);
    op.set_aead("None");

    /* Setting password */
    if(!password.empty())
        op.set_password(password.c_str(), RNP_ALGNAME_SHA256, 0, RNP_ALGNAME_AES_256);   

    rnp_key_handle_destroy(key);
    key = nullptr;

    if (op.execute() != RNP_SUCCESS) 
        return "Failed to encrypt.\n";

    return true;
}
