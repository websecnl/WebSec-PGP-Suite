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
