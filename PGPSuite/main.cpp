#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#define RNP_NO_DEPRECATED
#include <rnp/rnp.h>

constexpr int RNP_SUCCESS{ 0 };

/*
* rnp_input_from // create an input object which takes an input from some source
* rnp_output_from // create an output object which outputs something to the indicated location
*/

/*
* create a primary and subkey where the primary key is in charge of signing and the subkey of encrypting
*
    'primary': {                    the primary key
        'type': 'RSA',              assymetric algorithm used to generate it
        'length': 2048,             length in bits of primary key
        'userid': 'rsa@key',        user id used to locate said key
        'expiration': 31536000,     time till the key expires
        'usage': ['sign'],          ?? what to do with the key i suppose ??
        'protection': {             the preferred symmetric cipher and hasher
            'cipher': 'AES256',     preferred algorithms used for the symmetric cipher
            'hash': 'SHA256'        preferred hash algorithm to be used for signing
        }
    },
    'sub': {                        subkey data
        'type': 'RSA',
        'length': 2048,
        'expiration': 15768000,
        'usage': ['encrypt'],
        'protection': {
            'cipher': 'AES256',
            'hash': 'SHA256'
        }
    }
}";
*/
//const char* RSA_KEY_DESC = "{\
//    'primary': {\
//        'type': 'RSA',\
//        'length': 2048,\
//        'userid': 'rsa@key',\
//        'expiration': 31536000,\
//        'usage': ['sign'],\
//        'protection': {\
//            'cipher': 'AES256',\
//            'hash': 'SHA256'\
//        }\
//    },\
//    'sub': {\
//        'type': 'RSA',\
//        'length': 2048,\
//        'expiration': 15768000,\
//        'usage': ['encrypt'],\
//        'protection': {\
//            'cipher': 'AES256',\
//            'hash': 'SHA256'\
//        }\
//    }\
//}";

const char* RSA_KEY_DESC = "{\
    'primary': {\
        'type': 'RSA',\
        'length': 2048,\
        'userid': 'rsa@key',\
        'expiration': 31536000,\
        'usage': ['sign'],\
        'protection': {\
            'cipher': 'AES256',\
            'hash': 'SHA256'\
        }\
    },\
    'sub': {\
        'type': 'RSA',\
        'length': 2048,\
        'expiration': 15768000,\
        'usage': ['encrypt'],\
        'protection': {\
            'cipher': 'AES256',\
            'hash': 'SHA256'\
        }\
    }\
}";

bool example_pass_provider( rnp_ffi_t           ffi,
                            void*               app_ctx,
                            rnp_key_handle_t    key,
                            const char*         pgp_context,
                            char                buf[],
                            size_t              buf_len)
{
    if (strcmp(pgp_context, "protect")) {
        return false;
    }

    // strncpy(buf, "password", buf_len);
    strncpy_s(buf, buf_len, "password", buf_len);
    return true;
}

bool generate_keys()
{
    rnp_ffi_t ffi = nullptr; /* The context rnp works in */
    rnp_output_t output = nullptr; /* Stores where to output keys */
    char* key_grips = nullptr; /* Key buffer */
    std::unique_ptr<char> _grips = nullptr;

    if (rnp_ffi_create(&ffi, "GPG", "GPG") != RNP_SUCCESS)
    {
        std::cout << "Failed to create FFI context\n";
        return false;
    }

    rnp_ffi_set_pass_provider(ffi, example_pass_provider, nullptr);

    if (auto err = rnp_generate_key_json(ffi, RSA_KEY_DESC, &key_grips); 
        err != RNP_SUCCESS)
    {
        std::cout << "Failed to generate key from json\n";
        std::cout << "Error code: " << err << '\n';
        return false;
    }
    rnp_buffer_destroy(key_grips);
    key_grips = nullptr;

    if (rnp_output_to_path(&output, "pubring.pgp") != RNP_SUCCESS)
    {
        std::cout << "Failed to create output\n";
        rnp_output_destroy(output);
        return false;
    }

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        std::cout << "Failed to save keys\n";
        return false;
    }

    rnp_output_destroy(output);
    rnp_ffi_destroy(ffi);
}

std::string read_file(const std::string& filename)
{
    std::ifstream file(filename);
    std::stringstream buffer_stream;
    buffer_stream << file.rdbuf();
    return buffer_stream.str();
}

std::string prompt_input(std::string prompt)
{
    std::cout << prompt;
    std::string input_buffer;
    std::getline(std::cin, input_buffer);
    return input_buffer;
}

int main()
{
    /*std::string filename;
    std::getline(std::cin, filename);

    std::cout << read_file(filename);*/

    if (!generate_keys())
    {
        std::cerr << "Problem generating keys\n";
    }

    return 0;
}
