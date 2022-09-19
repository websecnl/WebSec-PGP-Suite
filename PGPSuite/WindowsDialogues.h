#pragma once

#include "OpenFileDiag.h"

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