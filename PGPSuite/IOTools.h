#pragma once

#include <fstream>
#include <sstream>
#include <string>

namespace io
{
    inline std::string read_file(const std::string& filename)
    {
        std::ifstream file(filename);
        std::stringstream buffer_stream;
        buffer_stream << file.rdbuf();
        return buffer_stream.str();
    }

    inline std::wstring read_file(const std::wstring& filename)
    {
        std::wifstream file(filename);
        std::wstringstream buffer_stream;
        buffer_stream << file.rdbuf();
        return buffer_stream.str();
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

