#include "PGPGenerateKeys.h"

bool pgp::generic_cin_pass_provider(rnp_ffi_t ffi, void* app_ctx, rnp_key_handle_t key, const char* pgp_context, char buf[], size_t buf_len)
{
    std::cout << "Nothing for no\n";
    std::string input = io::prompt_input(pgp_context, ": ");

    auto end = input.end();
    if (input.size() > buf_len)
    {
        end = input.begin() + (buf_len - 1);
        std::cout << "Input was truncated\n";
    }

    std::copy(input.begin(), end, buf);

    return input.size() > 0;
}

bool pgp::generate_keys(std::string pubkey_file, std::string secret_file, std::string key_data)
{
    rnp::FFI ffi("GPG", "GPG");
    rnp::Output output; /* where to save the keys */
    rnp::Buffer<char> key_grips; /* JSON result buffer */
    std::string json_data{};

    try
    {
        /* file has to exist, else just exit */
        json_data = io::read_file(key_data, true);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
        std::cerr << "\nDid you spell '" << key_data << "' correctly?\n";
        return false;
    }

    /* have to make proper pass provider for here */
    rnp_ffi_set_pass_provider(ffi, NULL/*example_pass_provider*/, nullptr);

    if (auto err = rnp_generate_key_json(ffi, json_data.c_str(), &key_grips.buffer);
        err != RNP_SUCCESS)
    {
        std::cerr << "Failed to generate key from json\n";
        std::cerr << "Error code: " << err << '\n';
        return false;
    }

    std::cout << "Json result: " << key_grips << '\n';
    key_grips.destroy();

    if (output.set_output_to_path(std::forward<std::string>(pubkey_file)) != RNP_SUCCESS) return false;

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to save keys\n";
        return false;
    }

    if (output.set_output_to_path(std::forward<std::string>(secret_file)) != RNP_SUCCESS) return false;

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_SECRET_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to save keys\n";
        return false;
    }

    return true;
}
