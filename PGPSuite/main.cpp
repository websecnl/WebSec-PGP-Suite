#include "PGPSuiteApplication.h"

/* std */
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

/* rnp */
#define RNP_NO_DEPRECATED
#include <rnp/rnp.h>

/* local */
#include "rnp_wrappers.h"

/* (NOTES)
* 
*  [ INPUTS and OUTPUTS ]
* rnp_input_from // create an input object which takes an input from some source
* rnp_output_from // create an output object which outputs something to the indicated location
* 
*  [ PASSWORDS ]
* when generating a key, a password can be set, this password is to be used when rnp prompts with: "password for key 'keyID' : "
* 
* this password will be asked when for example decrypting a message using the private key coupled to the public key used for encrypting the data
* 
* when encrypting a message a password can be set, this password is to be used when rnp prompts with: "Enter password to decrypt data : "
* 
* when placing a password on a message before encrypting, it allows someone to decrypt the data with either the password or the private key 
  thats coupled to the public key that encrypted it
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
        'protection': {             protection of the key
            'cipher': 'AES256',     symmetric algorithm used for the key passphrase
            'hash': 'SHA256'        hash algorithm used for the key passphrase
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

template<typename... _Args>
std::string prompt_input(_Args... prompt)
{
    ((std::cout << prompt), ...);
    std::string input_buffer;
    std::getline(std::cin, input_buffer);
    return input_buffer;
}

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

bool cin_pass_provider( rnp_ffi_t           ffi,
                        void*               app_ctx,
                        rnp_key_handle_t    key,
                        const char*         pgp_context,
                        char                buf[],
                        size_t              buf_len)
{
    std::string input{};
    if (pgp_context == std::string("decrypt (symmetric)") ||
        pgp_context == std::string("decrypt"))
        input = prompt_input(pgp_context, ": ");
       
    if (input.size() > buf_len) return false;

    std::copy(input.begin(), input.end(), buf);

    return input.size() > 0;
}

/*
* if (!strcmp(pgp_context, "decrypt (symmetric)")) {
        strncpy(buf, "encpassword", buf_len);
        return true;
    }
    if (!strcmp(pgp_context, "decrypt")) {
        strncpy(buf, "password", buf_len);
        return true;
    }
*/

bool generate_keys(std::string public_keyring = "pubring.pgp", std::string secret_keyring = "secring.pgp")
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
    
    if (output.set_output_to_path(public_keyring.c_str()) != RNP_SUCCESS) return false;

    if (rnp_save_keys(ffi, "GPG", output, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to save keys\n";
        return false;
    }

    if (output.set_output_to_path(secret_keyring.c_str()) != RNP_SUCCESS) return false;

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
    if (output_message.set_output_to_path("password_protected.asc") != RNP_SUCCESS) return false;

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
    op.set_password("cool-wachtwoord", RNP_ALGNAME_SHA256, 0, RNP_ALGNAME_AES_256);

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

bool decrypt_text(std::string secring_file = "secring.pgp", std::string encrypted_file = "message.asc")
{
    rnp::FFI ffi("GPG", "GPG");
    rnp::Input keyfile;
    rnp::Input input;
    rnp::Output output;
    std::vector<uint8_t> buffer;

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

    rnp_ffi_set_pass_provider(ffi, cin_pass_provider, NULL);

    /* create file input and memory output objects for the encrypted message and decrypted
     * message */
    if (input.set_input_from_path(std::forward<std::string>(encrypted_file)) != RNP_SUCCESS) return false;

    if (output.set_output_to_memory() != RNP_SUCCESS) return false;

    /* input: where is the encrypted data
       output: where to save the decrypted data */
    if (rnp_decrypt(ffi, input, output) != RNP_SUCCESS) {
        std::cout << "public-key decryption failed\n";
        return false;
    }

    /* get the decrypted message from the output structure */
    buffer = output.get_memory_buffer();

    std::cout << "The decrypted message: ";
    for (const auto c : buffer)
        std::cout << c;
    std::cout << '\n';

    return true;
}

std::string read_file(const std::string& filename)
{
    std::ifstream file(filename);
    std::stringstream buffer_stream;
    buffer_stream << file.rdbuf();
    return buffer_stream.str();
}

wxIMPLEMENT_APP(suite::MyApp);

//int main()
//{
//    /*std::string filename;
//    std::getline(std::cin, filename);
//
//    std::cout << read_file(filename);*/
//
//    // if (!generate_keys("other_pubring.pgp", "other_secring.pgp"))
//    // {
//    //     std::cerr << "Problem generating keys\n";
//    // }
//    // 
//    // if (!encrypt_text(prompt_input("Text to encrypt: ")))
//    //     std::cerr << "Problem encrypting text\n";
//    //  else 
//    //     std::cout << "Encrypted input\n";
//    
//    /*if (!decrypt_text("secring.pgp"))
//        std::cerr << "Problem decrypting data\n";
//    else
//        std::cout << "Decrypted data\n";*/
//
//    
//
//    return 0;
//}
