#pragma once

#include <Windows.h>
#include <string>
#include <algorithm>
#include <optional>

#include "pgpsuite_common.h"

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

    /* Applies a function to a determined list of variables */
    template<typename _Pred, typename ... _Args>
    bool are_all(_Pred pred, _Args&&... args)
    {
        return ( ( pred(std::forward<_Args>(args) ) ) && ...);
    }   

    /* Find first element in init list that when passed to pred returns False */
    template<typename _Type, typename _Pred> inline
    std::optional<_Type> find_if_not(std::initializer_list<_Type> types, _Pred pred)
    {
        auto found = std::find_if_not(types.begin(), types.end(), pred);
        return found == types.end() ? std::optional<_Type>{} : std::optional<_Type>(*found);
    }

    /* Checks if the given strings are in ascii and returns an error if one is not
    @note Will make a copy of the strings when an invalid one is found */
    template<class _String, typename ... _Args>
    OpRes validate_strings(_Args&&... strings)
    {
        if (!are_all(all_ascii<_String>, std::forward<_Args&&>(strings)... ))
        {
            auto invalid_type = find_if_not({ strings... }, all_ascii<_String>);
            return "Non valid characters detected in:\n" + *invalid_type + "\nMake sure all characters are ascii.";
        }
        return true;
    }

    /* Copy C++ types safely to C types, truncates if dest_size is less than source size */
    template<typename _Buffer> inline
    void copy_to_ctype(const _Buffer& source, char* dest, size_t dest_size)
    {
        auto end = source.end();
        if (source.size() > dest_size)
            end = source.begin() + (dest_size - 1);

        std::copy(source.begin(), end, dest);
    }
}

