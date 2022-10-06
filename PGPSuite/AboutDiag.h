#pragma once

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

namespace suite
{
    inline static const char* _about_text =
R"(Address:
WebSec B.V.
Keurenplein 41 A6260
1069 CD Amsterdam

Email: contact@websec.nl
Tel: 085-0023061

Application Info
Developer: Koen
Version: v1.0
Status: Latest Version
)";

    class AboutDiag :
        public wxDialog
    {
    protected:
        wxTextCtrl* text_control{ nullptr };
    public:
        AboutDiag(wxWindow* parent, wxWindowID id,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr))
            : wxDialog(parent, id, title, pos, size, style, name)
        {
            auto main_sizer = new wxBoxSizer(wxVERTICAL);

            text_control = new wxTextCtrl(this, wxID_ANY, _(_about_text), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
            main_sizer->Add(text_control, 1, wxGROW);
            
            auto button_sizer = CreateButtonSizer(wxOK);
            main_sizer->Add(button_sizer);

            text_control->SetFocus();

            SetSizer(main_sizer);
            SetMinSize(wxSize(300, 250));
            Fit();
        }
    };
}
