/*
 *
 * Copyright (c) 2018-2023
 * Author: WebSec B.V.
 * Developer: Koen Blok
 * Website: https://websec.nl
 *
 * Permission to use, copy, modify, distribute this software
 * and its documentation for non-commercial purposes is hereby granted exclusivley
 * under the terms of the GNU GPLv3 License.
 *
 * Most importantly:
 *  1. The above copyright notice appear in all copies and supporting documents.
 *  2. The application / code will not be used or reused for commercial purposes.
 *  3. All modifications are documented.
 *  4. All new releases will remain open source and contain the same license.
 *
 * WebSec B.V. makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * please read the full license agreement for more information:
 * https://github.com/websecnl/PGPSuite/LICENSE.md
 */
#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>

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

    template<typename _String>
    inline std::vector<char> read_file_bytes(const _String& filename)
    {
        auto filestream = std::ifstream(filename, std::ios::binary | std::ios::ate);
        auto file_position = filestream.tellg();

        auto bytes = std::vector<char>(file_position);

        filestream.seekg(0, std::ios::beg);
        filestream.read(&bytes[0], file_position);

        return bytes;
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

