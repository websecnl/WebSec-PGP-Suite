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

#include <wx/wxprec.h>

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
    some.thing.exe -> some.thing */
    inline std::string remove_extension(const std::string& str)
    {
        auto pos = str.find_last_of('.');

        /* return str if '.' not found
            if the '.' was at the start then just return the string */
        if (pos == str.npos || pos == 0) return str;

        return str.substr(0, pos);
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

    /* @brief Adds all found userid's to the given wxChoice object
    @param pubkey_fname: The key to derive userid's from
    @param choices: wxChoice object to add userid's to */
    inline bool add_keys_to_choice(std::string pubkey_fname, wxChoice* choices)
    {
        rnp::FFI ffi("GPG", "GPG");
        rnp::Input key_input{};
        rnp_key_handle_t key{};
        rnp_identifier_iterator_t it{};
        wxArrayString keyids{};
        char* keyid{ nullptr };

        key_input.set_input_from_path(std::move(pubkey_fname));
        if (auto res = rnp_load_keys(ffi, "GPG", key_input, RNP_LOAD_SAVE_PUBLIC_KEYS) != RNP_SUCCESS) return false;

        rnp_identifier_iterator_create(ffi, &it, "userid");
        while (true)
        {
            auto res = rnp_identifier_iterator_next(it, (const char**)&keyid);

            if (res != RNP_SUCCESS) return false;
            if (keyid == NULL) break;

            keyids.Add(_(keyid));
        }

        choices->Set(keyids);

        rnp_key_handle_destroy(key);
        rnp_buffer_destroy(keyid);
        rnp_identifier_iterator_destroy(it);

        return true;
    }
}

