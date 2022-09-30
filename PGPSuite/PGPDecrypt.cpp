#include "PGPDecrypt.h"

bool pgp::cin_pass_provider(rnp_ffi_t ffi, void* app_ctx, rnp_key_handle_t key, const char* pgp_context, char buf[], size_t buf_len)
{
    std::string input{};
    if (pgp_context == std::string("decrypt (symmetric)") ||
        pgp_context == std::string("decrypt"))
        input = io::prompt_input(pgp_context, ": ");

    auto end = input.end();
    if (input.size() > buf_len)
    {
        end = input.begin() + (buf_len - 1);
        std::cout << "Input was truncated\n";
    }

    std::copy(input.begin(), end, buf);

    return input.size() > 0;
}

bool pgp::decrypt_text(std::string secring_file, std::string encrypted_file, std::string output_fname, rnp_password_cb passprovider)
{
    rnp::FFI ffi("GPG", "GPG");
    rnp::Input keyfile;
    rnp::Input input;
    rnp::Output output;
    std::vector<uint8_t> buffer;

    if (output_fname.size() == 0)
        output_fname = utils::remove_extension(encrypted_file);

    /* load secret keyring, as it is required for public-key decryption. However, you may
        * need to load public keyring as well to validate key's signatures. */
    if (keyfile.set_input_from_path(std::forward<std::string>(secring_file))) return false;

    /* we may use RNP_LOAD_SAVE_SECRET_KEYS | RNP_LOAD_SAVE_PUBLIC_KEYS as well*/
    if (rnp_load_keys(ffi, "GPG", keyfile, RNP_LOAD_SAVE_SECRET_KEYS) != RNP_SUCCESS)
    {
        std::cout << "failed to read secring.pgp\n";
        return false;
    }
    keyfile.destroy();

    rnp_ffi_set_pass_provider(ffi, passprovider, NULL);

    /* create file input and memory output objects for the encrypted message and decrypted
     * message */
    if (input.set_input_from_path(std::forward<std::string>(encrypted_file)) != RNP_SUCCESS) return false;

    if (output.set_output_to_path(std::forward<std::string>(output_fname)) != RNP_SUCCESS) return false;

    /* input: where is the encrypted data
       output: where to save the decrypted data */
    if (rnp_decrypt(ffi, input, output) != RNP_SUCCESS) {
        std::cerr << "public-key decryption failed\n";
        return false;
    }

    return true;
}
