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
}
