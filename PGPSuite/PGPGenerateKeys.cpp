#include "PGPGenerateKeys.h"

bool pgp::generic_cin_pass_provider(rnp_ffi_t ffi, void* app_ctx, rnp_key_handle_t key, const char* pgp_context, char buf[], size_t buf_len)
{
    /* when generating the key the first time this is prompted is when
    the user is asked to write a password for their secret key
    the second time is to unlock their secret which already is unlocked
    so we skip the second call */
    static bool skip_next_call{ false };

    if (skip_next_call)
    {/* if user generates another key we dont wanna skip that */
        skip_next_call = false;
        return true;
    }

    std::cout << "Write password to protect key\n";
    std::string input = io::prompt_input(pgp_context, ": ");

    auto end = input.end();
    if (input.size() > buf_len)
    {
        end = input.begin() + (buf_len - 1);
        std::cout << "Input was truncated\n";
    }

    std::copy(input.begin(), end, buf);

    skip_next_call = true;

    return input.size() > 0;
}

pgp::OpRes pgp::generate_keys(std::string pubkey_file, std::string secret_file, std::string_view key_settings, rnp_password_cb passprovider)
{
    rnp::FFI ffi("GPG", "GPG");
    rnp::Output output; /* where to save the keys */
    rnp::Buffer<char> key_grips; /* JSON result buffer */

    if (auto res = pgp::utils::validate_strings<std::string>(pubkey_file, secret_file); !res) return res;

    /* Have to make proper pass provider for here */
    rnp_ffi_set_pass_provider(ffi, passprovider, nullptr);

    /* Check this first to be able to provide the user with a more clear error message */
    if (!pgp::utils::all_ascii(key_settings)) return "Non-ascii characters in JSON data.\n";

    if (auto err = rnp_generate_key_json(ffi, key_settings.data(), &key_grips.buffer);
        err != RNP_SUCCESS)
    {
        return "Failed to generate key from json.\n";
    }

    key_grips.destroy();

    if (output.set_output_to_path(std::forward<std::string>(pubkey_file)) != RNP_SUCCESS) return "Failed to set output.";

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        return "Failed to save keys\n";
    }

    if (output.set_output_to_path(std::forward<std::string>(secret_file)) != RNP_SUCCESS) return "Failed to set output.";

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_SECRET_KEYS) != RNP_SUCCESS)
    {
        return "Failed to save keys\n";
    }

    return true;
}
