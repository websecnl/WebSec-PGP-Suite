#pragma once

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

namespace suite
{
    /* meant as a modal dialogue to display a notepad like text editor */
    class TextEditDiag :
        public wxDialog
    {
    protected:
        wxTextCtrl* text_control{};
    public:
        TextEditDiag(wxWindow* parent, wxWindowID id,
            const wxString& title,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxDefaultSize,
            long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER,
            const wxString& name = wxASCII_STR(wxDialogNameStr))
            : wxDialog(parent, id, title, pos, size, style, name)
        {
            auto main_sizer = new wxBoxSizer(wxVERTICAL);

            text_control = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
            main_sizer->Add(text_control, 1, wxGROW);

            auto button_sizer = CreateButtonSizer(wxOK);
            main_sizer->Add(button_sizer);

            text_control->SetFocus();

            SetSizer(main_sizer);
            SetMinSize(wxSize(300, 250));
            Fit();
        }

        void reset()
        {
            text_control->Clear();
        }

        void set_value(const wxString& val)
        {
            text_control->SetValue(val);
        }

        wxString get_value() const
        {
            return text_control->GetValue();
        }
    };
}
