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

namespace io
{
    /* Spawn a dialog and returns its value */
    inline wxString text_prompt(wxString prompt, wxString prompt_desc)
    {
        wxTextEntryDialog dialog(nullptr, prompt_desc, prompt, wxEmptyString, wxOK | wxCANCEL);
    
        if (dialog.ShowModal() != wxID_OK)
        {
            wxMessageBox(_("Operation halted"), _("Cancelled"));
            return _("");
        }
    
        return dialog.GetValue();
    }

    /* Spawn file selection window */
    inline wxString file_select_prompt(wxWindow* parent, const char* wildcard = "All files|*", long style = wxFD_OPEN | wxFD_FILE_MUST_EXIST)
    {
        wxFileDialog openFileDialog(parent, _("Open file"), "", "", _(wildcard), style);

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return _("");

        return openFileDialog.GetPath();
    }

    /* specialized functions for retrieving items from wx containers */
    template<typename _Type> inline
    wxString wxget_value(_Type* of) { return _(""); }

    template<> inline
    wxString wxget_value<wxTextCtrl>(wxTextCtrl* ctrl)
    {
        return ctrl->GetValue();
    }

    template<> inline
    wxString wxget_value<wxChoice>(wxChoice* ctrl)
    {
        const auto sel = ctrl->GetSelection();
        return ctrl->GetString(sel);
    }
}
