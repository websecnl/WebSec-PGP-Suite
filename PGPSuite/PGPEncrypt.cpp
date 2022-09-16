#include "PGPEncrypt.h"

bool pgp::encrypt_text(uint8_t* data, size_t size, std::string save_to, bool add_password)
{
    rnp::Input input_message;
    rnp::Output output_message;
    rnp_key_handle_t key = nullptr;

    rnp::Input input_key;
    rnp::FFI ffi("GPG", "GPG");

    /* Load key file */
    if (input_key.set_input_from_path("pubring.pgp") != RNP_SUCCESS) return false;

    /* Load the to be encrypted message */
    if (input_message.set_input_from_memory(data, size, false) != RNP_SUCCESS) return false;

    /* Prepare the output for the encrypted message */
    if (output_message.set_output_to_path(std::forward<std::string>(save_to)) != RNP_SUCCESS) return false;

    rnp::EncryptOperation op(ffi, input_message, output_message);

    /* Attempt to read pubring.pgp for its keys */
    if (rnp_load_keys(ffi, "GPG", input_key, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to read pubring.pgp\n";
        return false;
    }

    /* Set encryption parameters */
    op.set_armor(true);
    op.set_file_name("message.txt");
    op.set_file_mtime(time(NULL));
    op.set_compression("ZIP", 6);
    op.set_cipher(RNP_ALGNAME_AES_256);
    op.set_aead("None");

    /* Setting password */
    if(add_password)
        op.set_password(io::prompt_input("Password: ").c_str(), RNP_ALGNAME_SHA256, 0, RNP_ALGNAME_AES_256);

    /* Locate key using the userid and load it into the key_handle_t */
    if (rnp_locate_key(ffi, "userid", "rsa@key", &key) != RNP_SUCCESS)
    {
        std::cerr << "failed to locate recipient key rsa@key\n";
        return false;
    }

    /* Recipient public key */
    if (op.add_recipient(key) != RNP_SUCCESS) return false;

    rnp_key_handle_destroy(key);
    key = nullptr;

    if (op.execute() != RNP_SUCCESS) return false;

    return true;
}
