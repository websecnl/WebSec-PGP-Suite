#pragma once

#include <Windows.h>
#include <string>
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace suite::reg
{
    inline static std::wstring docname{ L"PGPSuite compatible file" };
    inline static std::wstring suitename{ L"PGPSuite_file" };
    
    inline auto executable_path()
    {
        wxFileName f(wxStandardPaths::Get().GetExecutablePath());
        return f.GetFullPath();
    }

    inline bool register_write_path(std::wstring name, std::wstring description, std::wstring exec_path)
    {
        HKEY  hKey, hRootKey;
        DWORD dwDisp;
        long  nRet;

        wxString command(L"\"" + executable_path() + L"\" \"%1\"");
        
        std::wstring suitepath{ name + L"\\shell\\open\\command" };

        nRet = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, suitepath.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisp);

        if (nRet == ERROR_SUCCESS)
        {
            // Write the value for new document
            ::RegOpenKeyEx(HKEY_CLASSES_ROOT, name.c_str(), 0, KEY_ALL_ACCESS, &hRootKey);
            ::RegSetValueEx(hRootKey, nullptr, 0, REG_SZ, reinterpret_cast<LPBYTE>(description.data()), (description.size() + 1) * sizeof(std::wstring::value_type));
            RegCloseKey(hRootKey);

            ::RegSetValueEx(hKey, nullptr, 0, REG_SZ, (const LPBYTE)exec_path.data(), (command.size() + 1) * sizeof(std::wstring::value_type));
            RegCloseKey(hKey);

            return true;
        }
        
        RegCloseKey(hKey);

        return false;
    }

    /* adds context menu command for all files, if use icon is an empty string it wont be constructed */
    inline bool add_context_menu_command(std::wstring exec_path, std::wstring menu_text, std::wstring use_icon)
    {
        HKEY key{};
        DWORD disp{};
        long ret_val{};

        ret_val = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"*\\shell", 0, KEY_ALL_ACCESS, &key);

        if (ret_val == ERROR_SUCCESS)
        {
            /* create menutext key with value for icon */
            RegCreateKeyExW(key, menu_text.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, &disp);
            
            if (!use_icon.empty())
                ret_val = RegSetValueExW(key, L"Icon", 0, REG_SZ, (const LPBYTE)use_icon.data(), (use_icon.size() + 1) * sizeof(std::wstring::value_type));

            /* create command to run when user clicks the command */
            RegCreateKeyExW(key, L"command", 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, &disp);
            ret_val = RegSetValueExW(key, nullptr, 0, REG_SZ, (const LPBYTE)exec_path.data(), (exec_path.size() + 1) * sizeof(std::wstring::value_type));
            
            RegCloseKey(key);

            return ret_val == ERROR_SUCCESS;
        }

        RegCloseKey(key);

        return false;
    }

    /* adds context menu command for files with extension ext, if use icon is an empty string it wont be constructed */
    inline bool add_context_menu_command(std::wstring exec_path, std::wstring ext, std::wstring menu_text, std::wstring use_icon)
    {
        HKEY key{};
        DWORD disp{};
        long ret_val{};

        ret_val = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"SystemFileAssociations", 0, KEY_ALL_ACCESS, &key);

        if (ret_val == ERROR_SUCCESS)
        {
            const auto command_text = ext + L"\\Shell\\" + menu_text;

            /* create menutext key with value for icon */
            RegCreateKeyExW(key, command_text.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, &disp);
            
            if (!use_icon.empty())
                ret_val = RegSetValueExW(key, L"Icon", 0, REG_SZ, (const LPBYTE)use_icon.data(), (use_icon.size() + 1) * sizeof(std::wstring::value_type));

            /* create command to run when user clicks the command */
            RegCreateKeyExW(key, L"command", 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, &disp);
            ret_val = RegSetValueExW(key, nullptr, 0, REG_SZ, (const LPBYTE)exec_path.data(), (exec_path.size() + 1) * sizeof(std::wstring::value_type));
            
            RegCloseKey(key);

            return ret_val == ERROR_SUCCESS;
        }

        RegCloseKey(key);

        return false;
    }

    /* remove context menu command for files with extension ext */
    inline bool remove_context_menu_command(std::wstring menu_text, std::wstring ext)
    {
        HKEY key{};
        DWORD disp{};
        long ret_val{};

        ret_val = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"SystemFileAssociations", 0, KEY_ALL_ACCESS, &key);

        /* could remove recursively but can already see that going wrong so hardcoded it will be */
        auto command = L"\\SystemFileAssociations\\" + ext + L"\\Shell\\" + menu_text + L"\\command";
        auto command_text = L"\\SystemFileAssociations\\" + ext + L"\\Shell\\" + menu_text;
        if (ret_val == ERROR_SUCCESS)
        {
            RegDeleteKey(HKEY_CLASSES_ROOT, command.c_str());
            RegDeleteKey(HKEY_CLASSES_ROOT, command_text.c_str());

            RegCloseKey(key);

            return true;
        }

        RegCloseKey(key);

        return false;
    }

    /* remove context menu command for all files */
    inline bool remove_context_menu_command(std::wstring menu_text)
    {
        HKEY key{};
        DWORD disp{};
        long ret_val{};
        
        auto command = L"\\*\\shell\\" + menu_text + L"\\command";
        auto command_text = L"\\*\\shell\\" + menu_text;
        ret_val = RegOpenKeyExW(HKEY_CLASSES_ROOT, command.c_str(), 0, KEY_ALL_ACCESS, &key);
        
        if (ret_val == ERROR_SUCCESS)
        {
            RegDeleteKey(HKEY_CLASSES_ROOT, command.c_str());
            RegDeleteKey(HKEY_CLASSES_ROOT, command_text.c_str());
            
            RegCloseKey(key);

            return true;
        }
        
        RegCloseKey(key);

        return false;
    }

    inline bool register_for_extension(std::wstring ext)
    {
        HKEY hKey;
        DWORD dwDisp;
        long nRet;

        std::wstring suitebackup{ L"PGPSuite_backup" };

        nRet = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, ext.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisp);

        if (nRet == ERROR_SUCCESS)
        {
            TCHAR valData[MAX_PATH];
            DWORD valDataLen = MAX_PATH * sizeof(TCHAR);

            if (dwDisp == REG_OPENED_EXISTING_KEY)
            { /* if key exists, grab old key*/
                int res = ::RegQueryValueEx(hKey, L"", nullptr, nullptr, reinterpret_cast<LPBYTE>(valData), &valDataLen);
                if (res == ERROR_SUCCESS) /* and copy it under suitebackup */
                    ::RegSetValueEx(hKey, suitebackup.c_str(), 0, REG_SZ, reinterpret_cast<LPBYTE>(valData), valDataLen);
            }
            auto res = ::RegSetValueEx(hKey, nullptr, 0, REG_SZ, reinterpret_cast<const BYTE*>(suitename.data()), (suitename.size() + 1) * sizeof(std::wstring::value_type));

            ::RegCloseKey(hKey);

            return res == ERROR_SUCCESS;
        }


        return false;
    }

    inline bool is_path_registered()
    {
        HKEY hKey;

        const auto res = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, L"PGPSuite_file", 0, KEY_ALL_ACCESS, &hKey);

        RegCloseKey(hKey);

        return res == ERROR_SUCCESS;
    }

    inline bool is_user_admin()
    { /* Derived from Notepad++ GitHub */
        SID_IDENTIFIER_AUTHORITY nt_authority = SECURITY_NT_AUTHORITY;
        PSID admin_group;
        BOOL is_admin = AllocateAndInitializeSid(&nt_authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin_group);

        if (is_admin)
        {
            if (!CheckTokenMembership(NULL, admin_group, &is_admin))
                is_admin = FALSE;
            FreeSid(admin_group);
        }

        return is_admin == TRUE;
    }
    
    /* 
    @brief Registers .asc files to open with PGPSuite and adds context menu option for .asc files to decrypt with suite
    */
    inline bool register_pgpsuite_associations()
    {
        wxString icon_path = executable_path() + L",0";
        wxString base_command(L"\"" + executable_path() + L"\"");
        wxString perc1 = L" \"%1\"";
        wxString no_arg_command = base_command + perc1;
        wxString arg_command = base_command + L" \"-e\"" + perc1;

        bool a{ true }, b{ true }, c{ true }, d{ true };

        if (!is_path_registered())
            a = register_write_path(suitename, docname, no_arg_command.wc_str());
        b = register_for_extension(L".asc");
        c = add_context_menu_command(arg_command.wc_str(), L"Encrypt with PGPSuite", icon_path.wc_str());
        d = add_context_menu_command(no_arg_command.wc_str(), L".asc", L"Decrypt with PGPSuite", icon_path.wc_str());

        return a && b && c;
    }

    inline void remove_pgpsuite_associations()
    {
        remove_context_menu_command(L"Encrypt with PGPSuite");
        remove_context_menu_command(L"Decrypt with PGPSuite", L".asc");
    }
}
