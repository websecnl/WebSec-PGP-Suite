#pragma once

#include "CDialogEventHandler.h"

namespace io
{
    /* @brief Opens the windows file dialog
    @param selected_items: Upon successful exit, writes the selected items to here */
    inline
    HRESULT BasicFileOpen(std::vector<std::wstring>& selected_items)
    {
        IFileOpenDialog* pfd = NULL;

        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pfd));

        if (SUCCEEDED(hr))
        {
            // Create an event handling object, and hook it up to the dialog.
            IFileDialogEvents* pfde = NULL;
            hr = CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
            if (SUCCEEDED(hr))
            {
                /* Hook up the event handler */
                DWORD dwCookie;
                hr = pfd->Advise(pfde, &dwCookie);
                if (SUCCEEDED(hr))
                {
                    // Set the options on the dialog.
                    DWORD dwFlags;

                    /* copy old flags so we dont change anything */
                    hr = pfd->GetOptions(&dwFlags);
                    if (SUCCEEDED(hr))
                    {
                        // Allow multi select
                        hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | FOS_ALLOWMULTISELECT);
                        if (SUCCEEDED(hr))
                        {
                            /* probs a better way to do this, but cba now */
                            hr = pfd->SetFileTypes(1 /*ARRAYSIZE(c_rgSaveTypes)*/, c_rgSaveTypes + 3);
                            if (SUCCEEDED(hr))
                            {
                                /* Show dialog */
                                hr = pfd->Show(NULL);
                                if (SUCCEEDED(hr))
                                {
                                    /* Check what the user selected upon clicking 'open' */
                                    IShellItemArray* psiaResults{ nullptr };
                                    hr = pfd->GetResults(&psiaResults);
                                    if (SUCCEEDED(hr))
                                    {
                                        DWORD count{};
                                        psiaResults->GetCount(&count);
                                        for (DWORD i = 0; i < count; i++)
                                        {
                                            /* retrieve item at index */
                                            IShellItem* item{ nullptr };
                                            psiaResults->GetItemAt(i, &item);
                                            /* copy the path */
                                            LPWSTR path{};
                                            item->GetDisplayName(SIGDN_FILESYSPATH, &path);
                                            /* copy the path again to items */
                                            selected_items.push_back(std::wstring(path));
                                            /* delete local copy of path */
                                            CoTaskMemFree(path);
                                        }
                                    }
                                    psiaResults->Release();
                                }
                            }
                        }
                    }
                    /* Unhook event handler */
                    pfd->Unadvise(dwCookie);
                }
                pfde->Release();
            }
            pfd->Release();
        }
        return hr;
    }
}
