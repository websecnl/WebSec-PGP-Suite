#pragma once

#include <string>
#include <algorithm>
#include <Windows.h>

namespace pgp::utils
{
    /* conversions, thanks to: https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte */

    /* Convert a wide Unicode string to an UTF8 string */
    inline std::string utf8_encode(const std::wstring& wstr)
    {
        if (wstr.empty()) return std::string();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
    }

    /* Convert an UTF8 string to a wide Unicode String */
    inline std::wstring utf8_decode(const std::string& str)
    {
        if (str.empty()) return std::wstring();
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
        std::wstring wstrTo(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
        return wstrTo;
    }

    /* @brief Will remove anything after the first '.' encountered
    something.exe -> something
    some.thing.exe -> some */
    inline std::string remove_extension(std::string& str)
    {
        auto itr = std::find(str.begin(), str.end(), '.');

        /* return str if '.' not found */
        if (itr == str.end()) return str;
        /* if the '.' was a the start then just return 'a' default value */
        if (itr == str.begin()) return "a";

        return std::string(str.begin(), itr);
    }

    /* Check if the given string contains only ascii characters */
    template<class _String> inline
        bool all_ascii(const _String& str)
    {
        return std::all_of(str.begin(), str.end(), isascii);
    }
}

