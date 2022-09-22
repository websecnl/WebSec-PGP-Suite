#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>

#include "Application.h"
#include "PGPGenerateKeys.h"
#include "IOTools.h"
#include "WindowsDialogues.h"

namespace suite
{
    enum class SuiteStates { Main, GenerateKey };

    /* GUI to handle key generation */
    class PGPSuiteKeyGeneration
        : public Application
    {
    protected:
        int _width{}, _height{};
    public:
        PGPSuiteKeyGeneration(int width, int height)
            : _width(width)
            , _height(height)
        {}

        void start() override
        {
            _ui.add_element<ui::DynamicTextBox>(Rectangle{ 0.f, 100.f, 200.f,200.f }, 20.f, "Welcome to the key generation window! heere we generate keys");
        }
    };

    /* @brief Main GUI for simple and quick OpenPGP encryption */
    class PGPSuiteApplication :
        public Application
    {
    protected:
        using AppPtr = std::shared_ptr<Application>;
        using StateMachine = std::unordered_map<SuiteStates, AppPtr>;

        int _width{}, _height{};
        io::WindowsDialogues _win_diags;
        SuiteStates _state = SuiteStates::Main;
        StateMachine _statemachine;
        ui::Button _back_button;

        bool no_state_active() const { return _state == SuiteStates::Main; }
        AppPtr active_state() { return _statemachine[_state]; }
        const AppPtr active_state() const { return _statemachine.at(_state); }

        void on_input() override
        {
            if (no_state_active()) return;
            active_state()->input();
            _back_button.update(GetMousePosition());
        }
        
        void on_update() override
        {
            if (no_state_active()) return;
            active_state()->update();
        }

        void on_draw() const override
        {
            if (no_state_active()) return;
            active_state()->draw();
            _back_button.draw();
        }

        void init()
        {
            auto button = _ui.add_element<ui::Button>(Rectangle{ 50.f,50.f,200.f,100.f });
            button->bind(ui::ButtonState::LeftClicked, [this]() { switch_state(SuiteStates::GenerateKey); });
        }

        void switch_state(SuiteStates state)
        {
            if (state == SuiteStates::Main)
            {
                _ui.show();
            }
            else
            {
                assert(has_state(state));
                _ui.hide();
            }

            _state = state;
        }

        template<class _State>
        void add_state(SuiteStates state)
        {
            /* dont want to accidentally overwrite a state */
            assert(!has_state(state));

            _statemachine[state] = std::make_shared<_State>(_width, _height);
            _statemachine[state]->start();
        }
    public:
        /* @param width: Window width
        @param height: Window height*/
        PGPSuiteApplication(int width, int height) 
            : _height(height)
            , _width(width) 
            , _back_button({0.f,0.f,50.f,20.f})
        {
            _back_button.bind(ui::ButtonState::LeftClicked, [this]() { switch_state(SuiteStates::Main); });
        }

        void start() override
        {
            init();
        }

        bool has_state(SuiteStates state) const { return _statemachine.find(state) != _statemachine.end(); }
    };
}
