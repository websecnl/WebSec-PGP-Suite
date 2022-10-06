#pragma once

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "versioning.h"

#include <unordered_map>

namespace suite
{
    inline static std::unordered_map<wxString, wxString> about_info
    {
        {_("Address"), _("WebSec B.V.\nKeurenplein 41 A6260\n1069 CD Amsterdam")},
        {_("Contact"), _("\n")},
        {_("Email"), _("contact@websec.nl")},
        {_("Tel"), _("085 - 0023061")},
        {_("Application Info"), _("\n")},
        {_("Developer"), _("Koen Blok")},
        {_("Version"), ver::get_local_version() },
        {_("Status"), _("Not checked")},
    };

    class AboutDiag :
        public wxDialog
    {
    protected:
        wxTextCtrl* text_control{ nullptr };
        wxString about_text;       

        void parse(const std::unordered_map<wxString, wxString>& info)
        {
            for (const auto& [key, val] : info)
            {
                about_text += key + _(": ") + val + _("\n");
            }
        }
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

            parse(about_info);

            text_control = new wxTextCtrl(this, wxID_ANY, about_text, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
            main_sizer->Add(text_control, 1, wxGROW);
            
            auto button_sizer = CreateButtonSizer(wxOK);
            main_sizer->Add(button_sizer);

            text_control->SetFocus();

            SetSizer(main_sizer);
            SetMinSize(wxSize(300, 250));
            Fit();
        }

        void check_version()
        {
            const auto latest = ver::retrieve_version();
            const auto up_to_date = ver::compare(about_info["Version"], latest);

            about_info["Status"] = up_to_date ? "Up-to-date" : "New version available";

            parse(about_info);
        }
    };
}
