#pragma once

#include <iostream>
#include <string>
#include <optional>
#include <vector>
#include <assert.h>
#include <functional>

#define RNP_NO_DEPRECATED
#include <rnp/rnp.h>

#include "pgpsuite_common.h"

/* A collection of wrapper classes that utilize RAII to clean up the rnp C-objects
* The wrapper classes can all be cast to their original C-type 
* Rnp functions specific to a certain rnp type might be converted to member functions if applicable 
* For most if not all wrapper classes, the copy constructor is deleted */
namespace rnp
{
    /* IO Modes for the various IO classes */
    enum class IOMode { None, Memory, Path };

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

    /* Interface for the IO wrappers, handles deletion and io mode setting
    @param _IO type of the io object to be wrapped */
    template<typename _IO>
    struct IIOWrapper
    {
        using Destructor = std::function<void(_IO)>;

        IIOWrapper(Destructor func) : _destructor(func) {}
        IIOWrapper(const IIOWrapper&) = delete;
        ~IIOWrapper() { destroy(); }
        operator _IO() { return io_object; }

        void destroy()
        {
            if (is_io_set()) _destructor(io_object);
            _io_mode = IOMode::None;
        }

        /* The wrapped io object */
        _IO io_object{};

        /* @brief Check if the io has been set to any mode */
        bool is_io_set() const noexcept { return _io_mode != IOMode::None; }

        /* @brief check if the io is on a specific mode
        @param mode The mode to check for */
        bool is_io(IOMode mode) const { return _io_mode == mode; }
    protected:
        IOMode _io_mode{ IOMode::None };
        Destructor _destructor; /* function to destroy the io object */

        /* @brief Prepare input to be set, will delete old input
        @param mode The io mode that the input is preparing for */
        void prepare_io(IOMode mode)
        {
            if (is_io_set()) destroy(); /* If already opened, close old first */
            _io_mode = mode;
        }
    };

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
    * Also automatically destroys old output when setting new output
    * It outputs data from here to somewhere like a file*/
    struct Output
        : public IIOWrapper<rnp_output_t>
    {
        Output() : IIOWrapper(rnp_output_destroy) {}

        /* @return A copy of the data in the internal buffer */
        std::vector<uint8_t> get_memory_buffer()
        {
            assert(is_io(IOMode::Memory));

            uint8_t* ptr{ nullptr };
            size_t size{ 0 };

            rnp_output_memory_get_buf(io_object, &ptr, &size, false);

            /* For now we copy the internal buffer, there is a better way but this will do for now */
            std::vector<uint8_t> buffer;
            buffer.reserve(size);
            std::copy(ptr, ptr + size, std::back_inserter(buffer));

            return buffer;
        }

        /* @brief Initialize output to write to memory
        @param max_alloc maximum amount of bytes to write, 0 is infinite and the default */
        rnp_result_t set_output_to_memory(size_t max_alloc = 0)
        {
            prepare_io(IOMode::Memory);

            const auto res = rnp_output_to_memory(&io_object, max_alloc);
            validate_result(res, "Failed setting output to memory.");

            return res;
        }

        rnp_result_t set_output_to_path(std::string&& path)
        {
            prepare_io(IOMode::Path);

            const auto res = rnp_output_to_path(&io_object, path.c_str());
            validate_result(res, "Failed setting path to:", path, "does it exist?");

            return res;
        }
    };

    /* Simple rnp_input_t wrapper, automatically cleans itself up via RAII
    * Also automatically destroys old input when setting new input
    * It inputs data from somewhere to here */
    struct Input
        : public IIOWrapper<rnp_input_t>
    {
        Input() : IIOWrapper(rnp_input_destroy) {}

        /* @brief Opens the given path for loading data */
        rnp_result_t set_input_from_path(std::string && path)
        {
            prepare_io(IOMode::Path);
            
            const auto res = rnp_input_from_path(&io_object, path.c_str());
            validate_result(res, "Failed to open: '", path, "' does it exist?");

            return res;
        }

        rnp_result_t set_input_from_memory(const uint8_t* data, size_t size, bool copy = false)
        {
            prepare_io(IOMode::Memory);

            return rnp_input_from_memory(&io_object, data, size, copy);
        }
    };

    /* Wrapper for rnp buffers
    * Automatically deletes storage using RAII but NOT when assigned new memory
    * Smart pointers could be used but they do not allow access to the member which forfeits their use in rnp style funtions */
    template<typename _Type>
    struct Buffer
    {
        Buffer() = default;
        Buffer(const Buffer<_Type>&) = delete;
        Buffer(Buffer<_Type>&& other)
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
