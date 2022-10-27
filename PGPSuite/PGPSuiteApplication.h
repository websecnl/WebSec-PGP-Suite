#pragma once

#pragma warning( disable: C26812 )

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/filedlg.h>
#include <wx/artprov.h>
#include <wx/wfstream.h>
#include <wx/simplebook.h>
#include <wx/textdlg.h>
#include <wx/radiobox.h>

#include <unordered_map>

#include "PGPEncrypt.h"
#include "PGPGenerateKeys.h"
#include "PGPDecrypt.h"
#include "TextEditDiag.h"
#include "IOwx.h"
#include "resource.h"
#include "AboutDiag.h"
#include "enums.h"
#include "QuickPromptOperations.h"
#include "RegUtils.h"

namespace suite
{
    /* Embedded json settings */
    inline static const char* json_data = 
R"({
    'primary': {
        'type': 'RSA',
        'length': 2048,
        'userid': 'user@id',
        'expiration': 31536000,
        'usage': ['sign'],
        'protection': {
            'cipher': 'AES256',
            'hash': 'SHA256'
        }
    },
    'sub': {
        'type': 'RSA',
        'length': 2048,
        'expiration': 15768000,
        'usage': ['encrypt'],
        'protection': {
            'cipher': 'AES256',
            'hash': 'SHA256'
        }
    }
}
)";
       
    /* Encryption mode 
        - File mode reads file and encrypts content
        - Text mode encrypts the given data */
    enum class EncMode { File, Text };

    class MyFrame : public wxFrame
    {
    protected:
        /* query using the id label text associated with the name field */
        using TextFieldMap = std::unordered_map<const char*, wxTextCtrl*>;

        TextFieldMap _input_fields;
        EncMode _enc_mode{ EncMode::File };
        std::string _json_data = json_data;

        wxPanel* create_encryption_page(wxBookCtrlBase* parent);
        wxPanel* create_generate_page(wxBookCtrlBase* parent);
        wxPanel* create_decrypt_page(wxBookCtrlBase* parent);

        wxMenuBar* create_menu_bar();

        /* i prefer linking them at runtime, because lambda's */
        void runtime_bind_events(wxBookCtrlBase* notebook);
        void startup_version_check();
    public:
        MyFrame()
            : wxFrame(NULL, wxID_ANY, "PGPSuite")
        {
            auto menuBar = create_menu_bar();
            
            SetMenuBar(menuBar);

            wxNotebook* notebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(400, 300));

            auto mainSizer = new wxBoxSizer(wxVERTICAL);
                        
            auto encryptPanel = create_encryption_page(notebook);
            auto decryptPanel = create_decrypt_page(notebook);
            auto generatePanel = create_generate_page(notebook);

            notebook->AddPage(generatePanel, _("Generate"));
            notebook->AddPage(encryptPanel, _("Encrypt"));
            notebook->AddPage(decryptPanel, _("Decrypt"));
            notebook->Layout();

            notebook->SetSelection(2);

            mainSizer->Add(notebook, 1, wxEXPAND);

            SetSizer(mainSizer);

            mainSizer->Fit(this);

            CreateStatusBar();
            SetStatusText("Ready...");

            runtime_bind_events(notebook);
            
            startup_version_check();
        }
    private:
        void OnExit(wxCommandEvent& event);
    };

    inline void MyFrame::OnExit(wxCommandEvent& event)
    {
        Close(true);
    }

    class MyApp : public wxApp
    {
    public:
        virtual bool OnInit()
        {
            wxFrame* frame = nullptr;
            
            if (argc > 2)
                frame = new suite::EncryptFrame(argc, argv);
            else if (argc > 1)
                frame = new suite::DecryptFrame(argc, argv);
            else
                frame = new MyFrame;
    
            frame->SetIcon(wxIcon(_("MY_ICON")));
            frame->CenterOnScreen(wxBOTH);
            frame->Show(true);

            return true;
        }
    };
}
