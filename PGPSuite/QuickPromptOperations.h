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

#include "IOwx.h"
#include <wx/wxprec.h>
#include "rnp_wrappers.h"
#include "PGPDecrypt.h"
#include <wx/statline.h>
#include <unordered_map>

namespace suite
{
	namespace
	{
		enum IDs { ID_Decrypt = wxID_HIGHEST + 1, ID_ENCRYPT, ID_SELECT_KEYFILE, ID_CLEAR_PUBKEY, ID_NONE };	
		static constexpr int text_width{ 125 };

		/* @brief Helper function to create a type _Item with a static string to the left
		* @param static_text the static text to be set to the left of the item
		* @param args the arguments of which _Item will be constructed
		* @return the sizer with the static text and the constructed item
		*/
		template<typename _Item, typename ... _Args>
		inline std::pair<wxBoxSizer*, _Item*> create_formatted_item_with_text(wxPanel* parent, wxString static_text, _Args&&... args)
		{
			auto sizer = new wxBoxSizer(wxHORIZONTAL);
			auto text = new wxStaticText(parent, wxID_ANY, static_text);
			auto item = new _Item(std::forward<_Args>(args)...);

			sizer->Add(text, 0, wxTOP | wxLEFT, 15);
			sizer->Add(item, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 15);
			text->SetMinSize(wxSize(text_width, text->GetMinSize().y));

			return { sizer, item };
		}

		/* @brief adds an input box with correct offsets and a button if button id is not ID_NONE
			@param map a map type with insert functionality
			@param parent parent panel
			@param sizer main sizer to add input to
			@param static_text text to place to the left of input box
			@param input_element enum with an id where to store the text input in _textfields
			@param button_id id to give the button, leave on ID_NONE for no button
			@return the sizer with the input box */
		template<class _Map, typename _Enum> 
		inline wxBoxSizer* create_input_box(_Map& map, wxPanel* parent, wxString static_text, _Enum input_element, wxButton* button = nullptr)
		{
			auto input_sizer = new wxBoxSizer(wxHORIZONTAL);
			auto text = new wxStaticText(parent, wxID_ANY, static_text);
			auto text_input = new wxTextCtrl(parent, wxID_ANY);

			input_sizer->Add(text, 0, wxTOP | wxLEFT, 15);

			if (button != nullptr)
			{
				input_sizer->Add(button, 0, wxTOP | wxLEFT, 15);
			}

			input_sizer->Add(text_input, 1, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 15);

			map.insert({ input_element, text_input });

			text->SetMinSize(wxSize(text_width, text->GetMinSize().y));

			return input_sizer;
		}

		/* 
		@param map map with wxTextControl's
		@param key key to query */
		template<typename _Map, typename _Key> inline
		std::string if_map_has(_Map& map, _Key key)
		{
			if (map.find(key) != map.end())
				return std::string(map[key]->GetValue().mb_str());
			return {};
		};
	}

	class EncryptFrame
		: public wxFrame
	{
	protected:
		enum class TextInput { PublicKey, KeyID, Password };

		std::unordered_map<TextInput, wxTextCtrl*> _textfields;
	public:
		EncryptFrame(int argc, wxCmdLineArgsArray& args)
			: wxFrame(NULL, wxID_ANY, "PGPSuite")
		{
			if (argc < 3) return;

			auto sizer = new wxBoxSizer(wxVERTICAL);
			SetSizer(sizer);
			
			auto panel = new wxPanel(this);
			sizer->Add(panel, 1, wxEXPAND, 15);

			auto panel_sizer = new wxBoxSizer(wxVERTICAL);
			panel->SetSizer(panel_sizer);

			auto filename = std::wstring(args[2].wc_str());

			auto* clear_file_button = new wxButton(panel, ID_SELECT_KEYFILE, _("File..."));
			auto* inputsizer = create_input_box(_textfields, panel, _("Public key"), TextInput::PublicKey, clear_file_button);
			panel_sizer->Add(inputsizer);	

			auto [choicesizer, choice] = create_formatted_item_with_text<wxChoice>(panel, _("KeyID"), panel, ID_KEYID_SELECTION);
			panel_sizer->Add(choicesizer);
			panel_sizer->Hide(choicesizer, true);
			choice->Disable();

			panel_sizer->Add(new wxStaticText(panel, wxID_ANY, _("or")), 0, wxALL, 5);

			inputsizer = create_input_box(_textfields, panel, _("Password"), TextInput::Password);
			panel_sizer->Add(inputsizer);

			auto button_sizer = new wxBoxSizer(wxHORIZONTAL);
			button_sizer->Add(new wxButton(panel, ID_ENCRYPT, _("Encrypt")), 0, wxTOP, 15);
			panel_sizer->Add(button_sizer);
			
			panel_sizer->Add(new wxStaticText(panel, wxID_ANY, _(L"©WebSec B.V. - All rights reserved")), 1, wxALIGN_CENTER_HORIZONTAL | wxTOP, 15);

			sizer->Fit(this);
			Layout();

			Bind(wxEVT_BUTTON, [this, filename, choice](wxCommandEvent&)
				{
					auto filedata = std::vector<char>{};
					auto password = if_map_has(_textfields, TextInput::Password);
					auto pub_key = if_map_has(_textfields, TextInput::PublicKey);
					
					if (password.empty() && pub_key.empty() && choice->IsEmpty())
					{
						wxMessageBox(_("Please provide a public key and keyid, password or both."), _("Error"));
						return;
					}

					try
					{
						filedata = io::read_file_bytes(filename);
					}
					catch (std::exception& e)
					{
						wxMessageBox(_("Could not open file: ") + filename, _("Could not open file"));
						return;
					}
					
					auto save_as_filename = pgp::utils::utf8_encode(filename) + ".asc";

					wxString keyid = choice->IsEmpty() ? _("") : io::wxget_value<wxChoice>(choice);
					const auto res = pgp::encrypt_text((uint8_t*)filedata.data(), filedata.size(), pub_key, std::string(keyid.mbc_str()), save_as_filename, password);

					if (res)
						wxMessageBox(_("Success"));
					else
						wxMessageBox(_("Encryption failed.\nReason: ") + res.what(), _("Error"));
				}, ID_ENCRYPT, ID_ENCRYPT);

			Bind(wxEVT_BUTTON, [this, clear_file_button, choice, panel_sizer, choicesizer](wxCommandEvent&)
				{
					choice->Clear();
					choice->Disable();
					_textfields[TextInput::PublicKey]->Clear();
					clear_file_button->SetLabelText(_("File..."));
					clear_file_button->SetId(ID_SELECT_KEYFILE);
				}, ID_CLEAR_PUBKEY, ID_CLEAR_PUBKEY);

			Bind(wxEVT_BUTTON, [this, clear_file_button, choice, panel_sizer, choicesizer](wxCommandEvent& e)
				{
					auto filename = io::file_select_prompt(this, "PGP file (*.pgp)|*.pgp| All files|*");

					if (filename.empty()) return; /* operation was cancelled */
					
					choice->Clear();
					choice->Disable();
					_textfields[TextInput::PublicKey]->Clear();
					if (!pgp::utils::add_keys_to_choice(std::string(filename.mbc_str()), choice))
					{
						wxMessageBox(_("Failed opening key."), _("Error"));

						if (panel_sizer->IsShown(choicesizer))
						{
							panel_sizer->Hide(choicesizer, true);
							Layout();
						}
						return;
					}
										
					panel_sizer->Show(choicesizer, true);
					_textfields[TextInput::PublicKey]->SetValue(std::move(filename));
					choice->Enable();
					choice->SetSelection(0);
					clear_file_button->SetLabelText(_("Clear"));
					clear_file_button->SetId(ID_CLEAR_PUBKEY);
					Layout();
				}, ID_SELECT_KEYFILE, ID_SELECT_KEYFILE);
		}
	};

	class DecryptFrame
		: public wxFrame
	{
	protected:
		enum class TextInput { SecretKey, Password };

		std::unordered_map<TextInput, wxTextCtrl*> _textfields;
	public:
		DecryptFrame(int argc, wxCmdLineArgsArray& args)
			: wxFrame(NULL, wxID_ANY, "PGPSuite")
		{
			if (argc < 2) return;

			auto sizer = new wxBoxSizer(wxVERTICAL);
			SetSizer(sizer);
			
			auto panel = new wxPanel(this);
			sizer->Add(panel, 1, wxEXPAND, 15);
			
			auto filename = std::string(args[1].mb_str());
			rnp::PacketInfo info(filename);

			auto main_sizer = new wxBoxSizer(wxVERTICAL);
			panel->SetSizer(main_sizer);

			if (info.key_protected())
			{
				auto key_sizer = create_input_box(_textfields, panel, _("Secret key: "), TextInput::SecretKey, new wxButton(panel, ID_SELECT_KEYFILE, _("file...")));

				main_sizer->Add(key_sizer, 1, wxEXPAND | wxALL ^ wxBOTTOM);

				if (info.password_protected())
					main_sizer->Add(new wxStaticText(panel, wxID_ANY, _("or")), 0, wxALL, 5);
			}
			
			if (info.password_protected())
			{
				auto password_sizer = create_input_box(_textfields, panel, _("Password: "), TextInput::Password);

				main_sizer->Add(password_sizer, 1, wxEXPAND | wxALL ^ wxBOTTOM);
			}

			if (!info.password_protected() && !info.key_protected())
			{
				wxMessageBox(_("This is not a compatible .asc file."), _("Error"));
				Destroy();
				return;
			}

			auto button_sizer = new wxBoxSizer(wxHORIZONTAL);
			button_sizer->Add(new wxButton(panel, ID_Decrypt, _("Decrypt")), 0, wxTOP, 15);
			main_sizer->Add(button_sizer);
			
			main_sizer->Add(new wxStaticText(panel, wxID_ANY, _(L"©WebSec B.V. - All rights reserved")), 1, wxALIGN_CENTER_HORIZONTAL | wxTOP, 15);

			sizer->Fit(this);

			auto passprovider = [](rnp_ffi_t, void* context, rnp_key_handle_t, const char* pgp_context, char buf[], size_t buf_len)
			{
				auto password = std::string{};

				if (strcmp(pgp_context, "decrypt (symmetric)") == 0)
					password = *static_cast<std::string*>(context);
				else
					password = io::text_prompt(_("Password"), rnp::get_password_acquisition_reason(pgp_context));
				
				pgp::utils::copy_to_ctype(password, buf, buf_len);

				return password.size() != 0;
			};

			/* if map has key, return string version of value */
			auto string_if_map_has = [this](TextInput key) -> std::string
			{
				if (_textfields.find(key) != _textfields.end())
					return std::string(_textfields[key]->GetValue().mb_str());
				return {};
			};
			
			Bind(wxEVT_BUTTON, [this, packet = info, filename, passprovider, string_if_map_has] (wxCommandEvent&)
				{
					auto password = string_if_map_has(TextInput::Password);
					auto secret_key = string_if_map_has(TextInput::SecretKey);

					const bool forgot_pass = packet.password_protected() && password.empty();
					const bool forgot_key = packet.key_protected() && secret_key.empty();
					const bool forgot_both = forgot_pass && forgot_key;

					if (forgot_both)
					{
						wxMessageBox(packet.both() ? _("Please fill one of the boxes") : _("Please fill in all boxes"));
						return;
					}

					const auto res = pgp::decrypt_text(filename, "", passprovider, password.size() > 0 ? &password : NULL, secret_key);

					if (res)
						wxMessageBox(_("Successfully decrypted data.\n") + _("Saved decrypted data to: ") + _(pgp::utils::remove_extension(filename)), _("Success!"));
					else
						wxMessageBox(_(res.what()), _("Error"), wxICON_ERROR);
				}, ID_Decrypt, ID_Decrypt);

			Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
				{
					auto input_filename = _textfields[TextInput::SecretKey];

					wxFileDialog openFileDialog(this, _("Open file"), "", "", _("PGP file (*.pgp)|*.pgp| All files|*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);

					if (openFileDialog.ShowModal() == wxID_CANCEL)
						return;

					input_filename->SetValue(openFileDialog.GetPath());
				}, ID_SELECT_KEYFILE, ID_SELECT_KEYFILE);
		}
	};
}
