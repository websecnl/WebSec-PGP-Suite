#pragma once

#include <iostream>
#include <string>
#include <optional>

#define RNP_NO_DEPRECATED
#include <rnp/rnp.h>

/* A collection of wrapper classes that utilize RAII to clean up the rnp C-objects
* The wrapper classes can all be cast to their original C-type 
* Rnp functions specific to a certain rnp type might be converted to member functions if applicable 
* For most if not all wrapper classes, the copy constructor is deleted */
namespace rnp
{
    /* Determines wether an rnp function was successfull */
    constexpr int RNP_SUCCESS{ 0 };

    namespace
    {
        /* Validate result gotten from an rnp function */
        bool validate_result(int result)
        {
            return result == RNP_SUCCESS;
        }

        /*Validate result gotten from an rnp function
        * @param result result code to check
        * @param print_on_failure strings to print on failure */
        template<typename... _Args>
        bool validate_result(int result, _Args... print_on_failure)
        {
            if (validate_result(result)) return true;
            ((std::cerr << print_on_failure << ' '), ...);
            std::cerr << "Error code: " << result << '\n';
            return false;
        }
    }

    /* Simple ffi wrapper to handle automatic clean up */
    struct FFI
    {
        FFI(std::string pub_format, std::string sec_format)
        {
            rnp_ffi_create(&ffi, pub_format.c_str(), sec_format.c_str());
        }
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
            prepare_input();

            const auto res = rnp_output_to_path(&output, path.c_str());
            validate_result(res, "Failed setting path to:", path, "does it exist?");

            return res;
        }

        bool is_output_set() const noexcept { return _output_set; }
    protected:
        bool _output_set{ false }; /* Keep track of opened state, will be true when in use */
        /* Prepare input to be set, will delete old input */
        void prepare_input()
        {
            if (is_output_set()) destroy(); /* If already opened, close old first */
            _output_set = true;
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

    /* Wrapper of rnp_input_t
    * Holds information regarding inputting data */
    struct Input
    {
        Input() = default;
        explicit Input(rnp_input_t in) : input(in) {}
        Input(const Input&) = delete;
        ~Input() { destroy(); }
        operator rnp_input_t() { return input; }

        rnp_input_t input{ nullptr };

        void destroy()
        {
            rnp_input_destroy(input);
            _input_set = false;
        }

        rnp_result_t set_input_from_path(std::string && path)
        {
            prepare_input();

            const auto res = rnp_input_from_path(&input, path.c_str());
            validate_result(res, "Failed to open:", path, "does it exist?");

            return res;
        }

        rnp_result_t set_input_from_memory(const uint8_t* data, size_t size, bool copy = false)
        {
            prepare_input();

            return rnp_input_from_memory(&input, data, size, copy);
        }

        bool is_input_set() const noexcept { return _input_set; }
    protected:
        bool _input_set{ false }; /* Keep track of opened state, will be true when in use */

        /* Clean up old input if there was */
        void prepare_input()
        {
            if (is_input_set()) destroy(); /* If already opened, close old first */
            _input_set = true;
        }
    };

    /* Wrapper of rnp_op_encrypt_t 
    * Holds information necessary for starting encryption operations */
    struct EncryptOperation
    {
        EncryptOperation() = default;
        EncryptOperation(FFI& ffi, Input& input, Output& output)
        {
            create(ffi, input, output);
        }
        EncryptOperation(const EncryptOperation&) = delete;
        ~EncryptOperation() { destroy(); }

        rnp_op_encrypt_t op{ nullptr };

        /* Will throw upon failure to create 
        *  If object was already created the old one will be destroyed */
        void create(FFI& ffi, Input& input, Output& output)
        {
            destroy();
            if (rnp_op_encrypt_create(&op, ffi, input, output) != RNP_SUCCESS)
                throw std::exception("Failed to create Encryption Operation");
        }

        void destroy()
        {
            if (op == nullptr) return;
            rnp_op_encrypt_destroy(op);
            op = nullptr;
        }

        /* Setters */
        /* Set wether output will be set to binary or plaintext */
        void set_armor(bool val) { rnp_op_encrypt_set_armor(op, val); }
        /* Set internal filename to be used, not filename of output message */
        void set_file_name(std::string&& path) { rnp_op_encrypt_set_file_name(op, path.c_str()); }
        /* Set the internally stored time for the data being encrypted */
        void set_file_mtime(uint32_t t) { rnp_op_encrypt_set_file_mtime(op, t); }
        /* Set the compression algorithm to be used and the level of compression which has a range of 0 - 9*/
        void set_compression(std::string&& compression, int level) { rnp_op_encrypt_set_compression(op, compression.c_str(), level); }
        /* Set the encryption algorithm */
        void set_cipher(std::string&& cipher) { rnp_op_encrypt_set_cipher(op, cipher.c_str()); }
        /* Set aead mode, disabled by default */
        void set_aead(std::string&& alg) { rnp_op_encrypt_set_aead(op, alg.c_str()); }

        /* Add recipient key to encrypting context */
        rnp_result_t add_recipient(rnp_key_handle_t key)
        {
            const auto res = rnp_op_encrypt_add_recipient(op, key);
            validate_result(res, "Error adding recipient");
            return res;
        }

        /**
         * @brief Add password which is used to encrypt data. Multiple passwords can be added.
         *
         * @param password NULL terminated password string, or NULL if password should be requested via password provider.
         * @param s2k_hash hash algorithm, used in key-from-password derivation. Pass NULL for default
         *        value. See rnp_op_encrypt_set_hash for possible values.
         * @param iterations number of iterations, used in key derivation function.
         *        According to RFC 4880, chapter 3.7.1.3, only 256 distinct values within the range
         *        [1024..0x3e00000] can be encoded. Thus, the number will be increased to the closest
         *        encodable value. In case it exceeds the maximum encodable value, it will be decreased
         *        to the maximum encodable value.
         *        If 0 is passed, an optimal number (greater or equal to 1024) will be calculated based
         *        on performance measurement.
         * @param s2k_cipher symmetric cipher, used for key encryption. Pass NULL for default value.
         * See rnp_op_encrypt_set_cipher for possible values.
         */
        rnp_result_t set_password(const char* password, const char* s2k_hash, size_t iterations, const char* s2k_cipher)
        {
            const auto res = rnp_op_encrypt_add_password(op, password, s2k_hash, iterations, s2k_cipher);
            validate_result(res, "Failed to set password");
            return res;
        }

        /* Execute encryption operation */
        rnp_result_t execute()
        {
            const auto res = rnp_op_encrypt_execute(op);
            validate_result(res, "Error executing encryption operation");
            return res;
        }
    };
}
