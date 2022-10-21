#pragma once

#include "IOwx.h"
#include <wx/wxprec.h>
#include "rnp_wrappers.h"
#include "PGPDecrypt.h"
#include <unordered_map>

namespace suite
{
	class DecryptFrame
		: public wxFrame
	{
	protected:
		enum class TextInput { SecretKey, Password };
		enum IDs { ID_Decrypt = wxID_HIGHEST + 1 };

		std::unordered_map<TextInput, wxTextCtrl*> _textfields;
	public:
		DecryptFrame(int argc, wxCmdLineArgsArray& args)
		{
			if (argc < 2) return;

			auto filename = std::string(args[1].mb_str());
			rnp::PacketInfo info(filename);

			auto main_sizer = new wxBoxSizer(wxVERTICAL);

			if (info.key_protected())
			{
				auto input_sizer = new wxBoxSizer(wxHORIZONTAL);
				auto text = new wxStaticText(this, wxID_ANY, _("Secret key: "));
				auto text_input = new wxTextCtrl(this, wxID_ANY);

				input_sizer->Add(text);
				input_sizer->Add(text_input);

				_textfields.insert({ TextInput::SecretKey, text_input });

				main_sizer->Add(input_sizer);
			}

			if (info.password_protected())
			{
				auto input_sizer = new wxBoxSizer(wxHORIZONTAL);
				auto text = new wxStaticText(this, wxID_ANY, _("Password: "));
				auto text_input = new wxTextCtrl(this, wxID_ANY);

				input_sizer->Add(text);
				input_sizer->Add(text_input);

				_textfields.insert({ TextInput::Password, text_input });

				main_sizer->Add(input_sizer);
			}

			main_sizer->Add(new wxButton(this, ID_Decrypt, _("Decrypt")));

			Fit();

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

			Bind(wxEVT_BUTTON, [this, filename, passprovider](wxCommandEvent&)
				{
					auto password = _textfields[TextInput::Password];
					auto key = _textfields[TextInput::SecretKey];

					auto str_password = std::string(password->GetValue().mb_str());

					/* dont forget to remove the allocated password */
					pgp::decrypt_text(filename, "", passprovider, &str_password, std::string(key->GetValue().mb_str()));
				}, ID_Decrypt, ID_Decrypt);
		}
	};

	class DecryptPrompt
		: public wxApp
	{
	public:
		virtual bool OnInit()
		{
			DecryptFrame* frame = new DecryptFrame(argc, argv);
			frame->SetIcon(wxIcon(_("MY_ICON")));
			frame->CenterOnScreen(wxBOTH);
			frame->Show(true);
			return true;
		}
	};
}
