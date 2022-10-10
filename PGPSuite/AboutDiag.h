#pragma once

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "versioning.h"
#include "enums.h"

#include <unordered_map>
#include <vector>

namespace suite
{
    namespace
    {
        /* Need this because unordered_map/map doesnt keep insertion order */
        inline static const std::vector<wxString> about_info_order
        {
            _("Address"), _("Contact"), _("Email"), _("Tel"), _("Application Info"), _("Developer"), _("Version"), _("Status"),
        };
    }

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
            about_text.clear();
            for (const auto& order_key : about_info_order)
            {
                const auto iter = info.find(order_key);
                if (iter == info.end()) 
                    /* Makes it obvious if i missed something */
                    wxMessageBox(_("UNKNOWN KEY IN ORDER"));
                else
                {
                    const auto& [key, val] = *info.find(order_key);
                    about_text += key + _(": ") + val + _("\n");
                }
            }
            text_control->SetValue(about_text);
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

            text_control = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
            main_sizer->Add(text_control, 1, wxGROW);
            
            auto button_sizer = CreateButtonSizer(wxOK);
            auto version_check = new wxButton(this, ID_CHECK_VERSION, _("Check version"));
            main_sizer->Add(button_sizer);
            button_sizer->Add(version_check);

            parse(about_info);

            text_control->SetFocus();

            SetSizer(main_sizer);
            SetMinSize(wxSize(300, 250));
            Fit();
            
            Bind(wxEVT_BUTTON, &AboutDiag::check_version, this, ID_CHECK_VERSION, ID_CHECK_VERSION);
        }

        void check_version(wxCommandEvent&)
        {
            const wxString latest = ver::retrieve_version();

            if (latest.size() == 0)
            {
                wxMessageBox(_("Could not reach host, check your internet connection."), _("Error."), wxICON_ERROR);
                return;
            }

            const auto up_to_date = ver::compare(about_info["Version"], latest);

            about_info["Status"] = up_to_date ? _("Up-to-date") : _("New version available");

            if(!up_to_date)
            {
                wxMessageBox(_("Current version: ") + about_info["Version"] + _("\nNew version: ") + latest, _("New version available."));
            }

            parse(about_info);
        }
    };
}
