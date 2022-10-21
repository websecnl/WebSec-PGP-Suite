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

pgp::OpRes pgp::decrypt_text(std::string encrypted_file, std::string output_fname, rnp_password_cb passprovider, void* context, std::string secring_file)
{
    rnp::FFI ffi("GPG", "GPG");
    rnp::Input input;
    rnp::Output output;
    std::vector<uint8_t> buffer;

    if (auto res = pgp::utils::validate_strings<std::string>(secring_file, encrypted_file, output_fname); !res) return res;

    if (output_fname.size() == 0)
        output_fname = utils::remove_extension(encrypted_file);

    /* if a secret keyring is provided, load it up */
    if (!secring_file.empty())
    {
        rnp::Input keyfile;

        /* load secret keyring, as it is required for public-key decryption. However, you may
            * need to load public keyring as well to validate key's signatures. */
        if (keyfile.set_input_from_path(secring_file)) 
            return "Failed setting input for: " + secring_file;

        if (rnp_load_keys(ffi, "GPG", keyfile, RNP_LOAD_SAVE_SECRET_KEYS) != RNP_SUCCESS)
            return "Failed to read secring.pgp\n";
    }

    rnp_ffi_set_pass_provider(ffi, passprovider, context);

    /* create file input and memory output objects for the encrypted message and decrypted
     * message */
    if (input.set_input_from_path(std::forward<std::string>(encrypted_file)) != RNP_SUCCESS) return "Error setting input\n";

    if (output.set_output_to_path(std::forward<std::string>(output_fname)) != RNP_SUCCESS) return "Error setting output\n";

    /* input: where is the encrypted data
       output: where to save the decrypted data */
    if (rnp_decrypt(ffi, input, output) != RNP_SUCCESS) 
    {
        return "Decryption failed\n";
    }

    return true;
}
