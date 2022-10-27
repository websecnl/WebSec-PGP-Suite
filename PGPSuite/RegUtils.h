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

    inline bool register_write_path()
    {
        HKEY  hKey, hRootKey;
        DWORD dwDisp;
        long  nRet;

        wxString command(L"\"" + executable_path() + L"\" \"%1\"");
        
        std::wstring suitepath{ suitename + L"\\shell\\open\\command" };

        nRet = ::RegCreateKeyEx(HKEY_CLASSES_ROOT, suitepath.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &hKey, &dwDisp);

        if (nRet == ERROR_SUCCESS)
        {
            // Write the value for new document
            ::RegOpenKeyEx(HKEY_CLASSES_ROOT, suitename.c_str(), 0, KEY_ALL_ACCESS, &hRootKey);
            ::RegSetValueEx(hRootKey, nullptr, 0, REG_SZ, reinterpret_cast<LPBYTE>(docname.data()), (docname.size() + 1) * sizeof(std::wstring::value_type));
            RegCloseKey(hRootKey);

            ::RegSetValueEx(hKey, nullptr, 0, REG_SZ, (const LPBYTE)command.data().AsWChar(), (command.size() + 1) * sizeof(wxString::value_type));
            RegCloseKey(hKey);

            return true;
        }

        return false;
    }

    inline bool add_context_menu_command(std::wstring exec_path, std::wstring ext, std::wstring menu_text)
    {
        HKEY key{};
        DWORD disp{};
        long ret_val{};
        
        ret_val = RegOpenKeyExW(HKEY_CLASSES_ROOT, L"SystemFileAssociations", 0, KEY_ALL_ACCESS, &key);

        if (ret_val == ERROR_SUCCESS)
        {
            const auto command_path = ext + L"\\Shell\\" + menu_text + L"\\command";
            
            RegCreateKeyExW(key, command_path.c_str(), 0, nullptr, 0, KEY_ALL_ACCESS, nullptr, &key, &disp);
            ret_val = RegSetValueExW(key, nullptr, 0, REG_SZ, (const LPBYTE)exec_path.data(), (exec_path.size() + 1) * sizeof(std::wstring::value_type));

            return ret_val == ERROR_SUCCESS;
        }

        return false;
    }

    inline bool register_for_extension(std::wstring ext)
    {
        HKEY hKey;
        DWORD dwDisp;
        long nRet;

        std::wstring suitename{ L"PGPSuite_file" };
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
}
