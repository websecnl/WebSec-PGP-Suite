#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

namespace io
{
    namespace
    {
        template<class _FStream>
        inline void validate_file(_FStream& file)
        {
            if (file.fail())
                throw std::exception("File does not exist.");
        }

        template<class _Str, class _FStream, class _SStream>
        inline _Str read_file(const _Str& filename, bool file_has_to_exist = false)
        {
            _FStream file(filename);
            if (file_has_to_exist) validate_file<_FStream>(file);
            _SStream buffer_stream;
            buffer_stream << file.rdbuf();
            return buffer_stream.str();
        }
    }
    
    inline std::string read_file(const std::string& filename, bool file_has_to_exist = false)
    {
        return read_file<std::string, std::ifstream, std::stringstream>(filename, file_has_to_exist);
    }

    inline std::wstring read_file(const std::wstring& filename, bool file_has_to_exist = false)
    {
        return read_file<std::wstring, std::wifstream, std::wstringstream>(filename, file_has_to_exist);
    }

    template<typename... _Args> inline
    std::string prompt_input(_Args... prompt)
    {
        ((std::cout << prompt), ...);
        std::string input_buffer;
        std::getline(std::cin, input_buffer);
        return input_buffer;
    }
}

