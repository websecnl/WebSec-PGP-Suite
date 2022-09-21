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

            button->bind(ui::ButtonState::LeftClicked, []() { std::cout << "hello world!\n"; });
        }
    public:
        /* @param width: Window width
        @param height: Window height*/
        PGPSuiteApplication(int width, int height) 
            : _height(height)
            , _width(width) 
        { init(); }
    };
}
