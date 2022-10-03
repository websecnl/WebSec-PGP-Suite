#include "PGPSuiteApplication.h"

using namespace suite; /* i will allow myself this luxury */

wxPanel* MyFrame::create_encryption_page(wxBookCtrlBase* parent)
{
    auto panel = new wxPanel(parent);

    auto panelMainSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panelMainSizer);

    /* set names */
    for (auto flags = wxEXPAND | wxALL;
        auto str : { "File to encrypt", "KeyID of recipient", "Recipient public key" })
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
            pickFile = new wxButton(panel, ID_OPEN_FILE, _("File..."));
            break;
        case 'R':
            pickFile = new wxButton(panel, ID_OPEN_PUBKEY, _("File..."));
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

    // auto saveButton = new wxButton(panel, ID_SAVE_FILE, _("Encrypt"));
    // buttonSizer->Add(saveButton);

    auto saveButton = new wxButton(panel, ID_ENCRYPT_FILE, _("Encrypt"));
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

    auto button = new wxButton(panel, ID_GENERATE_KEY, _("Generate"));
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
        auto str : { "File to decrypt", "Private key" })
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
            pickFile = new wxButton(panel, ID_OPEN_ENC_FILE, _("File..."));
            break;
        case 'P':
            pickFile = new wxButton(panel, ID_OPEN_SECKEY, _("File..."));
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

    auto encryptButton = new wxButton(panel, ID_DECRYPT_FILE, _("Decrypt"));
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
    auto bind_button_filediag = [this](const char* key, const char* wildcard = "All files|*")
    {
        auto input = _input_fields[key];
        
        wxFileDialog openFileDialog(this, _("Open file"), "", "", _(wildcard), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (openFileDialog.ShowModal() == wxID_CANCEL)
            return;

        input->SetValue(openFileDialog.GetPath());
    };

    Bind(wxEVT_BUTTON, std::bind(bind_button_filediag, "File to encrypt"), ID_OPEN_FILE, ID_OPEN_FILE);

    Bind(wxEVT_BUTTON, std::bind(bind_button_filediag, "Recipient public key", "PGP file (*.pgp)|*.pgp| All files|*"), ID_OPEN_PUBKEY, ID_OPEN_PUBKEY);

    Bind(wxEVT_BUTTON, std::bind(bind_button_filediag, "Private key", "PGP file (*.pgp)|*.pgp|All files|*"), ID_OPEN_SECKEY, ID_OPEN_SECKEY);
    
    Bind(wxEVT_BUTTON, std::bind(bind_button_filediag, "File to decrypt", "ASC file (*.asc)|*.asc|All files|*"), ID_OPEN_ENC_FILE, ID_OPEN_ENC_FILE);
    
    /* Generic passprovider to be send to the different operations, will generate appropriate prompts */
    auto passprovider = [](rnp_ffi_t, void*, rnp_key_handle_t, const char* pgp_context, char buf[], size_t buf_len) -> bool
    {
        /* change prompt if asked for key pass or for file pass */
        wxString prompt = _("Please enter a password"), prompt_desc{};
        
        if (strcmp(pgp_context, "protect"))
            prompt_desc = _("Provide a password to encrypt secret key\n");
        else if (strcmp(pgp_context, "unprotect"))
            prompt_desc = _("Provide password to decrypt secret key\n");
        else if (strcmp(pgp_context, "decrypt (symmetric)"))
            prompt_desc = _("Provide password of the encrypted message\n");
        else if (strcmp(pgp_context, "decrypt"))
            prompt_desc = _("Provide secret key password to decrypt the data\n");
        else
            prompt_desc = _("Unknown password acquisition found: '") + _(pgp_context) + _("'.\nPlease inform developer.\n");

        wxTextEntryDialog dialog(nullptr, prompt_desc, prompt, wxEmptyString, wxOK | wxCANCEL);

        if (dialog.ShowModal() != wxID_OK)
        {
            wxMessageBox(_("Operation halted"), _("Cancelled"));
            return false;
        }

        wxString input = dialog.GetValue();

        auto end = input.end();
        if (input.size() > buf_len)
            end = input.begin() + (buf_len - 1);

        std::copy(input.begin(), end, buf);

        return input.size() > 0;
    };

    /* simple lambda to check if all parameters have a size of member that returns a number greater than 0 */
    auto all_filled = [](auto ... params) -> bool { return ((params.size() > 0) && ...); };

    /* ------------------------------------- GENERATE ---------------------------------------------- */

    // (TODO) generate save as dialogues for saving pubring and secring
    // (TODO) set proper error handling with OpRes
    Bind(wxEVT_BUTTON, [this, passprovider](wxCommandEvent& e)
        {
            // auto input = _input_fields["Recipient public key"];
            // for now lets ignore the user input
            PushStatusText(_("Generating..."));
            
            const auto success = pgp::generate_keys("pubring.pgp", "secring.pgp", "keygen.json", passprovider);
            
            PopStatusText();

            if (success)
                wxMessageBox(_("Success!"), _("Successfully generated keypair!"));
            else
                wxMessageBox(_("Failed!"), _("Keypair generation failed."));

        }, ID_GENERATE_KEY, ID_GENERATE_KEY);

    /* ------------------------------------- ENCRYPT ---------------------------------------------- */
    
    Bind(wxEVT_BUTTON, [this, all_filled](wxCommandEvent& e)
        {
            auto pubkey = _input_fields["Recipient public key"]->GetValue();
            auto file = _input_fields["File to encrypt"]->GetValue();
            auto keyID = _input_fields["KeyID of recipient"]->GetValue();
            
            if (!all_filled(pubkey, file, keyID))
            {
                wxMessageBox(_("Fill in all boxes"), _("Encryption failed"));
                return;
            }

            std::string filename = std::string(file.mb_str()), filedata{};

            try
            {
                filedata = io::read_file(filename, true);
            }
            catch (std::exception& e)
            {
                wxMessageBox(_("Could not open file: ") + file, _("Could not open file"));
                return;
            }

            wxFileDialog fileDialog(this, _("Save encrypted data to"), "", _("message"), "ASC files(*.asc) | *.asc | All files | *", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

            if (fileDialog.ShowModal() == wxID_CANCEL)
            {
                wxMessageBox(_("No file selected to save data to"), _("Encryption failed"));
                return;
            }

            const auto success = pgp::encrypt_text((uint8_t*)filedata.data(), filedata.size(), std::string(pubkey.mb_str()), std::string(keyID.mb_str()), std::string(fileDialog.GetPath().mb_str()));

            if (success)
                wxMessageBox(_("Successfully encrypted data."), _("Success!"));
            else
                wxMessageBox(_(success.what()), _("Failed!"));

        }, ID_ENCRYPT_FILE, ID_ENCRYPT_FILE);

    /* ------------------------------------- DECRYPT ---------------------------------------------- */

    Bind(wxEVT_BUTTON, [this, passprovider, all_filled](wxCommandEvent& e)
        {
            auto seckey = _input_fields["Private key"]->GetValue();
            auto file = _input_fields["File to decrypt"]->GetValue();

            if (!all_filled(seckey, file))
            {
                wxMessageBox(_("Fill in all boxes"), _("Decryption failed"));
                return;
            }

            std::string filename = std::string(file.mb_str()), filedata{};

            try
            {
                filedata = io::read_file(filename, true);
            }
            catch (std::exception& e)
            {
                wxMessageBox(_("Could not open file: ") + file, _("Could not open file"));
                return;
            }

            wxFileDialog fileDialog(this, _("Save decrypted data to"), "", _("message"), "All files|*", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

            if (fileDialog.ShowModal() == wxID_CANCEL)
            {
                wxMessageBox(_("No file selected to save data to"), _("Decryption failed"));
                return;
            }

            const auto success = pgp::decrypt_text(std::string(seckey.mb_str()), std::string(file.mb_str()), "", passprovider);

            if (success)
                wxMessageBox(_("Successfully decrypted data."), _("Success!"));
            else
                wxMessageBox(_(success.what()), _("Failed!"));

        }, ID_DECRYPT_FILE, ID_DECRYPT_FILE);
}

