#pragma once

#include "Application.h"
#include <iostream>
#include "PGPGenerateKeys.h"
#include "IOTools.h"
#include "WindowsDialogues.h"

namespace suite
{
    /* @brief Main GUI for simple and quick OpenPGP encryption */
    class PGPSuiteApplication :
        public Application
    {
    protected:
        int _width{}, _height{};
        io::WindowsDialogues _win_diags;

        void on_update() override
        {

        }

        void init()
        {
            float button_width = 100.f, button_height = 40.f;
            
            auto button = _ui.add_element<ui::Button>(Rectangle{ 50.f,50.f, button_width, button_height });

            button->bind(ui::ButtonState::LeftClicked, [this]()
                {
                    bool result = pgp::generate_keys();
                    static std::string success = "Success!", failure = "Failed!", desc = "Saved keys to secring.pgp and keyring.pgp";
                    _win_diags.simple_pop_up(result ? success : failure, result ? desc : failure);
                });
        }
    public:
        /* @param width: Window width
        @param height: Window height*/
        PGPSuiteApplication(int width, int height) 
            : _height(height)
            , _width(width) 
        {}

        void start() override
        {
            init();
        }
    };
}
