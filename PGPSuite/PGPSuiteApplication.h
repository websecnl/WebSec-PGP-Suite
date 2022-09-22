#pragma once

#include "Application.h"
#include <iostream>

namespace suite
{
    /* @brief Main GUI for simple and quick OpenPGP encryption */
    class PGPSuiteApplication :
        public Application
    {
    protected:
        int _width{}, _height{};

        void on_update() override
        {

        }

        void init()
        {
            auto button = _ui.add_element<ui::Button>(Rectangle{ 20.f,20.f,50.f,20.f });

            auto textbox = _ui.add_element<ui::StaticInputBox>(Rectangle{ 20.f,50.f,500.f,40.f }, 10.f);

            auto text = _ui.add_element<ui::DynamicTextBox>(Rectangle{ 20.f,100.f,550.f,40.f }, 30.f, std::string("Hello world! Textbox here!"));

            button->bind(ui::ButtonState::LeftClicked, [textbox, text]() { text->text(textbox->copy_buffer()); });
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
