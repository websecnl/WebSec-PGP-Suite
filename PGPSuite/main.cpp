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

/* Abstraction classes that utilize RAII to clean up the rnp c-objects 
* The wrapper classes can all be cast to their original C-type */
namespace rnp
{
    /* Simple ffi wrapper to handle automatic clean up */
    struct FFI
    {
        FFI(std::string pub_format, std::string sec_format)
        { rnp_ffi_create(&ffi, pub_format.c_str(), sec_format.c_str()); }
        FFI(const FFI&) = delete;
        ~FFI() { destroy(); }

        rnp_ffi_t ffi = nullptr;
        operator bool() { return ffi != nullptr; }
        operator rnp_ffi_t() { return ffi; } /* To allow passing this object as its underlying C-type */

        void destroy()
        {
            if (ffi == nullptr) return;
            rnp_ffi_destroy(ffi);
            ffi = nullptr;
        }
    };

    /* Simple rnp_output_t wrapper, automatically cleans itself up via RAII
    * Also automatically destroys old output when setting new output */
    struct Output
    {
        Output() = default;
        Output(const Output&) = delete;
        ~Output() { destroy(); }
        operator rnp_output_t() { return output; }

        rnp_output_t output{ nullptr };

        void destroy()
        {
            rnp_output_destroy(output);
            _output_set = false;
        }

        rnp_result_t set_output_to_path(std::string&& path)
        {
            if (is_output_set()) destroy(); /* If already opened, close old first */
            _output_set = true;
            
            const auto res = rnp_output_to_path(&output, path.c_str());
            is_valid_result(std::forward<std::string>(path), res);
            
            return res;
        }

        bool is_output_set() const noexcept { return _output_set; }
    protected:
        bool _output_set{ false }; /* Keep track of opened state, will be true when in use */

        static bool is_valid_result(std::string&& path, rnp_result_t result)
        {
            if (result == RNP_SUCCESS) return true;
            std::cerr << "Error setting path to: " << path << '\n';
            std::cerr << "Error code: " << result << '\n';
            return false;
        }
    };

    /* Wrapper for rnp buffers
    * Automatically deletes storage using RAII but NOT when assigned new memory
    * Smart pointers could be used but they do not allow access to the member which forfeits their use in rnp style funtions */
    template<typename _Type>
    struct Buffer
    {
        Buffer() = default;
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&& other)
        {
            destroy();
            buffer = other.buffer;
            other.buffer = nullptr;
        }
        ~Buffer() { destroy(); }

        _Type* buffer{ nullptr };

        void destroy() 
        { 
            if (buffer == nullptr) return;
            rnp_buffer_destroy(buffer);
            buffer = nullptr;
        }

        void clear(size_t size) { rnp_buffer_clear(buffer, size); }

        friend std::ostream& operator<<(std::ostream& out, const Buffer<_Type>& rhs)
        {
            if (rhs.buffer == nullptr) return out;
            return out << rhs.buffer;
        }
    };
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
    rnp_input_t input_message = nullptr;
    rnp_output_t output_message = nullptr;
    rnp_key_handle_t key = nullptr;
    rnp_op_encrypt_t encrypt_operation = nullptr;

    rnp_input_t input_key = nullptr;
    rnp::FFI ffi("GPG", "GPG");

    /* Load key file */
    if (rnp_input_from_path(&input_key, "pubring.pgp"))
    {
        std::cerr << "Failed to open pubring.pgp, does file exist?\n";
        return false;
    }

    /* Load the to be encrypted message */
    if (rnp_input_from_memory(&input_message, (uint8_t*)message.data(), message.size(), false) != RNP_SUCCESS)
    {
        std::cerr << "Error creating input from memory\n";
        return false;
    }

    /* Prepare the output for the encrypted message */
    if (rnp_output_to_path(&output_message, "message.asc"))
    {
        std::cerr << "Error creating output to file\n";
        return false;
    }

    /* Attempt to read pubring.pgp for its keys */
    if (rnp_load_keys(ffi, "GPG", input_key, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS)
    {
        std::cerr << "Failed to read pubring.pgp\n";
        return false;
    }

    /* Create encryption operation */
    if (rnp_op_encrypt_create(&encrypt_operation, ffi, input_message, output_message) != RNP_SUCCESS)
    {
        std::cerr << "Failed to create encryption operation\n";
        return false;
    }

    /* Set encryption parameters */
    rnp_op_encrypt_set_armor(encrypt_operation, true);
    rnp_op_encrypt_set_file_name(encrypt_operation, "message.txt");
    rnp_op_encrypt_set_file_mtime(encrypt_operation, time(NULL));
    rnp_op_encrypt_set_compression(encrypt_operation, "ZIP", 6);
    rnp_op_encrypt_set_cipher(encrypt_operation, RNP_ALGNAME_AES_256);
    rnp_op_encrypt_set_aead(encrypt_operation, "None");

    /* Locate key using the userid and load it into the key_handle_t */
    if (rnp_locate_key(ffi, "userid", "rsa@key", &key) != RNP_SUCCESS)
    {
        std::cerr << "failed to locate recipient key rsa@key\n";
        return false;
    }

    /* Recipient public key */
    if (rnp_op_encrypt_add_recipient(encrypt_operation, key) != RNP_SUCCESS)
    {
        std::cerr << "Failed to add recipient to key\n";
        return false;
    }
    rnp_key_handle_destroy(key);
    key = nullptr;

    if (rnp_op_encrypt_execute(encrypt_operation) != RNP_SUCCESS)
    {
        std::cerr << "Failed to execute encryption operation\n";
        return false;
    }
    rnp_op_encrypt_destroy(encrypt_operation);
    encrypt_operation = nullptr;

    /* Clean up */
    rnp_input_destroy(input_message);
    rnp_input_destroy(input_key);
    rnp_output_destroy(output_message);

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

    if (!generate_keys())
    {
        std::cerr << "Problem generating keys\n";
    }

    if (!encrypt_text( prompt_input("Text to encrypt: ") ))
    {
        std::cerr << "Problem encrypting text\n";
    }

    return 0;
}
