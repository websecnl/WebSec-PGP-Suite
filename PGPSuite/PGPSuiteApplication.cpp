#include "PGPSuiteApplication.h"

using namespace suite; /* i will allow myself this luxury */

wxPanel* MyFrame::create_encryption_page(wxBookCtrlBase* parent)
{
    auto panel = new wxPanel(parent);

    auto panelMainSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panelMainSizer);

    /* set names */
    for (auto flags = wxEXPAND | wxALL;
        auto str : { "File to encrypt", "Recipient public key" })
    {
        auto nameSizer = new wxBoxSizer(wxHORIZONTAL);
        panelMainSizer->Add(nameSizer, 1, flags, 15);

        auto name = new wxTextCtrl(panel, wxID_ANY);
        auto nameText = new wxStaticText(panel, wxID_ANY, _(str));
        nameSizer->Add(nameText);

        wxButton* pickFile = nullptr;

        switch (str[0])
        {
        case 'F':
            pickFile = new wxButton(panel, ID_PICK_FILE_TO_ENCRYPT, _("File..."));
            break;
        case 'R':
            pickFile = new wxButton(panel, ID_PICK_PUBKEY_FILE, _("File..."));
            break;
        }

        if (pickFile)
            nameSizer->Add(pickFile);

        nameText->SetMinSize(wxSize(125, nameText->GetMinSize().y));

        nameSizer->Add(name, 1);

        if (flags & wxTOP)
            flags ^= wxTOP; // remove top border after first iteration

        _input_fields[str] = name;
    }

    auto buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    panelMainSizer->Add(buttonSizer);

    auto saveButton = new wxButton(panel, ID_SAVE_FILE, _("Encrypt"));
    buttonSizer->Add(saveButton);

    return panel;
}

wxPanel* MyFrame::create_generate_page(wxBookCtrlBase* parent)
{
    auto panel = new wxPanel(parent);

    auto panelMainSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panelMainSizer);

    auto inputSizer = new wxBoxSizer(wxHORIZONTAL);
    panelMainSizer->Add(inputSizer, 1, wxEXPAND | wxALL, 15);

    auto keyText = new wxStaticText(panel, wxID_ANY, _("Key ID"));
    auto keyInput = new wxTextCtrl(panel, wxID_ANY);
    keyText->SetMinSize(wxSize(125, keyText->GetMinSize().y));
    inputSizer->Add(keyText);
    inputSizer->Add(keyInput);

    auto button = new wxButton(panel, wxID_ANY, _("Generate"));
    panelMainSizer->Add(button);

    return panel;
}

wxPanel* MyFrame::create_decrypt_page(wxBookCtrlBase* parent)
{
    auto panel = new wxPanel(parent);

    auto panelMainSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panelMainSizer);

    /* set names */
    for (auto flags = wxEXPAND | wxALL;
        auto str : { "File to decrypt", "Key ID", "Private key" })
    {
        auto nameSizer = new wxBoxSizer(wxHORIZONTAL);
        panelMainSizer->Add(nameSizer, 1, flags, 15);

        auto name = new wxTextCtrl(panel, wxID_ANY);
        auto nameText = new wxStaticText(panel, wxID_ANY, _(str));
        nameSizer->Add(nameText);

        wxButton* pickFile = nullptr;

        switch (str[0])
        {
        case 'F':
            pickFile = new wxButton(panel, ID_PICK_FILE_TO_ENCRYPT, _("File..."));
            break;
        case 'P':
            pickFile = new wxButton(panel, ID_PICK_PUBKEY_FILE, _("File..."));
            break;
        }

        if (pickFile)
            nameSizer->Add(pickFile);

        nameText->SetMinSize(wxSize(125, nameText->GetMinSize().y));

        nameSizer->Add(name, 1);

        _input_fields[str] = name;
    }

    auto buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    panelMainSizer->Add(buttonSizer);

    auto encryptButton = new wxButton(panel, ID_SAVE_FILE, _("Encrypt"));
    buttonSizer->Add(encryptButton);

    return panel;
}

wxMenuBar* MyFrame::create_menu_bar()
{
    wxMenu* menuFile = new wxMenu;
    menuFile->Append(ID_Hello, "&Hello...\tCtrl-H",
        "Help string shown in status bar for this menu item");
    menuFile->AppendSeparator();
    menuFile->Append(wxID_EXIT);

    wxMenu* menuHelp = new wxMenu;
    menuHelp->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    menuBar->Append(menuHelp, "&Help");

    return menuBar;
}

void MyFrame::runtime_bind_events(wxBookCtrlBase* notebook)
{
    Bind(wxEVT_TOOL, [notebook](wxCommandEvent&)
        {
            int cur_idx = notebook->GetSelection();
            notebook->SetSelection(!cur_idx);
        }, ID_My_Man, ID_My_Man);

    /* save to file */
    Bind(wxEVT_BUTTON, [this](wxCommandEvent& e)
        {
            // verify_encryption_requirements(); // this would just check if the boxes are filled

            wxFileDialog
                saveFileDialog(this, _("Save ASC file"), "", _("message"),
                    "ASC files (*.asc)|*.asc", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

            if (saveFileDialog.ShowModal() == wxID_CANCEL) return;

            wxFileOutputStream output_stream(saveFileDialog.GetPath());
            if (!output_stream.IsOk())
            {
                wxMessageBox(_("Failed to save the file!"), _("Failed!"));
                wxLogError("Cannot save current contents in file '%s'.", saveFileDialog.GetPath());
                return;
            }

            auto input = _input_fields["File to encrypt"]->GetValue();

            output_stream.WriteAll(input.c_str(), input.size());

            output_stream.Close();

            wxMessageBox(_("Successfully saved file!"), _("Success!"));
        }, ID_SAVE_FILE, ID_SAVE_FILE);

    /* these can be bound to the same function */
    Bind(wxEVT_BUTTON, [this](wxCommandEvent& e)
        {
            auto input = _input_fields["File to encrypt"];

            wxFileDialog openFileDialog(this, _("Open file"), "", "", "*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

            if (openFileDialog.ShowModal() == wxID_CANCEL)
                return;     // the user changed idea...

            input->SetValue(openFileDialog.GetPath());
        }, ID_PICK_FILE_TO_ENCRYPT, ID_PICK_FILE_TO_ENCRYPT);

    Bind(wxEVT_BUTTON, [this](wxCommandEvent& e)
        {
            auto input = _input_fields["Recipient public key"];

            wxFileDialog openFileDialog(this, _("Open file"), "", "", "*", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

            if (openFileDialog.ShowModal() == wxID_CANCEL)
                return;     // the user changed idea...

            input->SetValue(openFileDialog.GetPath());
        }, ID_PICK_PUBKEY_FILE, ID_PICK_PUBKEY_FILE);
}
