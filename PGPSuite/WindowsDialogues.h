#pragma once

#include "OpenFileDiag.h"
#include "Utils.h"

namespace io
{
    /* Class to use the windows dialogues like the file explorer */
    class WindowsDialogues
    {
    public:
        WindowsDialogues()
        {
            auto hr = CoInitialize(NULL);
        }

        void simple_pop_up(std::string& title_text, std::string& info_text)
        {
            TaskDialog(NULL,
                NULL,
                pgp::utils::utf8_decode(title_text).c_str(),
                pgp::utils::utf8_decode(info_text).c_str(),
                NULL,
                TDCBF_OK_BUTTON,
                TD_INFORMATION_ICON,
                NULL);
        }

        std::vector<std::wstring> multi_file_dialogue()
        {
            std::vector<std::wstring> filenames;
            multi_file_dialogue(filenames);
            return filenames;
        }

        void multi_file_dialogue(std::vector<std::wstring>& filenames)
        {
            io::windowsapi::MultiFileOpen(filenames);
        }

        std::wstring file_dialogue()
        {
            std::wstring filename;
            file_dialogue(filename);
            return filename;
        }

        void file_dialogue(std::wstring& filename)
        {
            io::windowsapi::SingleFileOpen(filename);
        }
    };
}
