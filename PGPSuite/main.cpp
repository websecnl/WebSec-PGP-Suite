/* stl */
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

/* rnp */
#define RNP_NO_DEPRECATED
#include <rnp/rnp.h>

/* local */
#include "rnp_wrappers.h"

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
        'usage': ['sign'],          by convention top level keys are to be used for signing and subkeys for encryption src( https://www.rfc-editor.org/rfc/rfc4880#section-5.5.1.2 )
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
    rnp::FFI ffi("GPG", "GPG"); /* The context rnp works in */
    rnp::Output output; /* Stores where to output keys */
    rnp::Buffer<char> key_grips; /* JSON result buffer */

    rnp_ffi_set_pass_provider(ffi, example_pass_provider, nullptr);

    if (auto err = rnp_generate_key_json(ffi, RSA_KEY_DESC, &key_grips.buffer );
        err != RNP_SUCCESS)
    {
        std::cerr << "Failed to generate key from json\n";
        std::cerr << "Error code: " << err << '\n';
        return false;
    }

    std::cout << "Json result: " << key_grips << '\n';
    key_grips.destroy();
    
    if (output.set_output_to_path("pubring.pgp") != RNP_SUCCESS) return false;

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to save keys\n";
        return false;
    }

    if (output.set_output_to_path("secring.pgp") != RNP_SUCCESS) return false;

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_SECRET_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to save keys\n";
        return false;
    }
    return true;
}

bool encrypt_text(std::string message)
{
    rnp::Input input_message;
    rnp::Output output_message;
    rnp_key_handle_t key = nullptr;

    rnp::Input input_key;
    rnp::FFI ffi("GPG", "GPG");

    /* Load key file */
    if (input_key.set_input_from_path("pubring.pgp") != RNP_SUCCESS) return false;

    /* Load the to be encrypted message */
    if (input_message.set_input_from_memory((uint8_t*)message.data(), message.size(), false) != RNP_SUCCESS) return false;

    /* Prepare the output for the encrypted message */
    if (output_message.set_output_to_path("message.asc") != RNP_SUCCESS) return false;

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

    // if (!generate_keys())
    // {
    //     std::cerr << "Problem generating keys\n";
    // }

    if (!encrypt_text( prompt_input("Text to encrypt: ") ))
    {
        std::cerr << "Problem encrypting text\n";
    }

    return 0;
}
