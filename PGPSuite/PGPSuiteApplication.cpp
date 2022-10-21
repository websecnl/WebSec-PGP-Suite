#include "PGPSuiteApplication.h"

using namespace suite; /* i will allow myself this luxury */

wxPanel* MyFrame::create_encryption_page(wxBookCtrlBase* parent)
{
    auto panel = new wxPanel(parent);

    auto panelMainSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panelMainSizer);

    /* set names */
    for (auto flags = wxEXPAND | wxALL ^ wxBOTTOM;
        auto str : { "File to encrypt", "KeyID of recipient", "Recipient public key", "Password(optional)" })
    {
        auto mainInputSizer = new wxBoxSizer(wxVERTICAL);
        auto nameSizer = new wxBoxSizer(wxHORIZONTAL);
        mainInputSizer->Add(nameSizer, 1, flags, 15);
        panelMainSizer->Add(mainInputSizer, 1, flags, 15);

        auto name = new wxTextCtrl(panel, wxID_ANY);
        auto nameText = new wxStaticText(panel, wxID_ANY, _(str));
        nameSizer->Add(nameText);

        wxButton* pickFile = nullptr;

        switch (str[0])
        {
        case 'F':
        {
            pickFile = new wxButton(panel, ID_OPEN_FILE, _("File..."));
            name->SetExtraStyle(wxTE_MULTILINE);
            name->SetWindowStyle(wxTE_MULTILINE);
            name->SetWindowStyleFlag(wxTE_MULTILINE);

            wxArrayString choices;
            choices.Add("File");
            choices.Add("Text");

            auto* radiobox = new wxRadioBox(panel, ID_ENC_TYPE_RADIO_CHANGED, wxEmptyString, wxDefaultPosition, wxDefaultSize, choices);
            mainInputSizer->Add(radiobox, 0, wxALIGN_RIGHT);
            
            Bind(wxEVT_RADIOBOX, [this, pickFile, radiobox, name](wxCommandEvent& e)
                {
                    auto selection = radiobox->GetString(e.GetSelection());
                    
                    if (selection == _("File"))
                    {
                        pickFile->SetId(ID_OPEN_FILE);
                        _enc_mode = EncMode::File;
                    }
                    else
                    {
                        pickFile->SetId(ID_EDIT_TEXT);
                        _enc_mode = EncMode::Text;
                    }

                    pickFile->SetLabelText(selection + _("..."));

                }, ID_ENC_TYPE_RADIO_CHANGED, ID_ENC_TYPE_RADIO_CHANGED);
            break;
        }
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
    auto keyInput = new wxTextCtrl(panel, wxID_ANY, _("user@id"));
    keyInput->Disable();
    keyText->SetMinSize(wxSize(125, keyText->GetMinSize().y));
    inputSizer->Add(keyText);
    inputSizer->Add(keyInput);

    auto button_sizer = new wxBoxSizer(wxHORIZONTAL);
    panelMainSizer->Add(button_sizer);

    auto button = new wxButton(panel, ID_GENERATE_KEY, _("Generate"));
    auto advanced_button = new wxButton(panel, ID_SHOW_GENERATE_SETTINGS, _("Advanced..."));
    button_sizer->Add(button);
    button_sizer->Add(advanced_button);

    return panel;
}

wxPanel* MyFrame::create_decrypt_page(wxBookCtrlBase* parent)
{
    auto panel = new wxPanel(parent);

    auto panelMainSizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panelMainSizer);

    /* set names */
    for (auto flags = wxEXPAND | wxALL ^ wxBOTTOM;
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
    wxMenu* menu_file = new wxMenu;
    menu_file->Append(wxID_EXIT);

    wxMenu* menu_help = new wxMenu;
    menu_help->Append(wxID_ABOUT);

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(menu_file, "&File");
    menuBar->Append(menu_help, "&Help");

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
        wxString 
            prompt = _("Please enter a password"), 
            prompt_desc = rnp::get_password_acquisition_reason(pgp_context);
                
        wxString input = io::text_prompt(prompt, prompt_desc);

        pgp::utils::copy_to_ctype(input, buf, buf_len);

        return input.size() > 0;
    };

    /* passprovider that cant be called twice in a row */
    auto passprovider_once = [](rnp_ffi_t, void*, rnp_key_handle_t, const char* pgp_context, char buf[], size_t buf_len) -> bool
    {
        static bool called_once = false;

        if (called_once)
        {
            called_once = false;
            return true;
        }

        wxString input = io::text_prompt(_("Please enter a password"), _("Provide a password to encrypt secret key.\n"));

        pgp::utils::copy_to_ctype(input, buf, buf_len);

        called_once = true;

        return input.size() > 0;
    };

    /* simple lambda to check if all parameters have a size of member that returns a number greater than 0 */
    auto all_filled = [](auto ... params) -> bool { return ((params.size() > 0) && ...); };

    /* ------------------------------------- GENERATE ---------------------------------------------- */

    // (TODO) generate save as dialogues for saving pubring and secring
    Bind(wxEVT_BUTTON, [this, passprovider_once](wxCommandEvent& e)
        {
            PushStatusText(_("Generating..."));
            
            const auto success = pgp::generate_keys("pubring.pgp", "secring.pgp", _json_data, passprovider_once);
            
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
            auto data = _input_fields["File to encrypt"]->GetValue();
            auto keyID = _input_fields["KeyID of recipient"]->GetValue();
            auto password = _input_fields["Password(optional)"]->GetValue();
                        
            if (!all_filled(data))
            {
                wxMessageBox(_("Provide data to encrypt."), _("Encryption failed"), wxICON_ERROR);
                return;
            }

            const bool has_keyid_and_pubkey = !keyID.empty() && !pubkey.empty();
            const bool has_password = !password.empty();

            if (!has_keyid_and_pubkey && !has_password)
            {
                wxMessageBox(_("Provide either a public key and keyID, a password or both."), _("Encryption failed"), wxICON_ERROR);
                return;
            }

            std::wstring filename = std::wstring(data.wc_str()), filedata{};

            if (_enc_mode == EncMode::File)
            { /* data is to be interpreted as file */
                try
                {
                    filedata = io::read_file(filename, true);
                }
                catch (std::exception& e)
                {
                    wxMessageBox(_("Could not open file: ") + data, _("Could not open file"));
                    return;
                }
            }
            else if (_enc_mode == EncMode::Text)
            { /* data is to be interpreted as string */
                filedata = std::wstring(data.wc_str());
            }
            
            wxFileDialog fileDialog(this, _("Save encrypted data to"), "", _("message"), "ASC files(*.asc) | *.asc | All files | *", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

            if (fileDialog.ShowModal() == wxID_CANCEL)
            {
                wxMessageBox(_("No file selected to save data to"), _("Encryption failed"));
                return;
            }

            const auto success = pgp::encrypt_text((uint8_t*)filedata.data(), filedata.size() * (sizeof(wchar_t) / sizeof(uint8_t)), 
                std::string(pubkey.mb_str()), std::string(keyID.mb_str()), std::string(fileDialog.GetPath().mb_str()), std::string(password.mb_str()));

            if (success)
                wxMessageBox(_("Successfully encrypted data."), _("Success!"));
            else
                wxMessageBox(_(success.what()), _("Failed!"));
        }, ID_ENCRYPT_FILE, ID_ENCRYPT_FILE);

    /* ------------------------------------- DECRYPT ---------------------------------------------- */
    /* (TODO) accept wstrings as filenames by opening the file and loading it into memory and then decrypting that */
    Bind(wxEVT_BUTTON, [this, passprovider, all_filled](wxCommandEvent& e)
        {
            auto seckey = _input_fields["Private key"]->GetValue();
            auto file = _input_fields["File to decrypt"]->GetValue();

            if (!all_filled(file))
            {
                wxMessageBox(_("Provide atleast a file to decrypt."), _("Decryption failed"));
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

            const auto success = pgp::decrypt_text(std::string(file.mb_str()), "", passprovider, NULL, std::string(seckey.mb_str()));

            if (success)
                wxMessageBox(_("Successfully decrypted data."), _("Success!"));
            else
                wxMessageBox(_(success.what()), _("Failed!"));
        }, ID_DECRYPT_FILE, ID_DECRYPT_FILE);
    
    /* ------------------------------------- TEXT EDIT ---------------------------------------------- */

    Bind(wxEVT_BUTTON, [this, passprovider, all_filled](wxCommandEvent& e)
        {
            auto input = _input_fields["File to encrypt"];
            
            auto text_dialog = suite::TextEditDiag(this, wxID_ANY, _("Text editor"));
            if (text_dialog.ShowModal() == wxID_OK)
            {
                input->SetValue(text_dialog.get_value());
                auto txt = text_dialog.get_value();
                auto c_txt = input->GetValue();
                return;
            }

        }, ID_EDIT_TEXT, ID_EDIT_TEXT);


    Bind(wxEVT_BUTTON, [this](wxCommandEvent& e)
        {
            TextEditDiag diag(this, wxID_ANY, _("Advanced settings"));
            diag.set_value(_json_data);

            if (diag.ShowModal() == wxID_OK)
            {
                _json_data = diag.get_value();
                return;
            }

        }, ID_SHOW_GENERATE_SETTINGS, ID_SHOW_GENERATE_SETTINGS);

    Bind(wxEVT_MENU, [this](wxCommandEvent& e)
        {
            AboutDiag diag = AboutDiag(this, wxID_ANY, _("About"));
            diag.ShowModal();
        }, wxID_ABOUT, wxID_ABOUT);
}

void suite::MyFrame::startup_version_check()
{
    const bool perform_check = persistent::settings().get("version").get("startup_check") == "yes";

    if (!perform_check) return;

    ver::verify_local_version();
}

