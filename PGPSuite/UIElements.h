#pragma once

#include <raylib.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>

namespace suite::ui::helpers
{
	inline bool rectangle_v_point(Rectangle a, Vector2 b)
	{
		return (b.x >= a.x && b.x <= a.x + a.width)
			&& (b.y >= a.y && b.y <= a.y + a.height);
	}
}

namespace suite::ui
{
	class UIElement
	{
	protected:
		Rectangle _transform;
	public:
		UIElement(Rectangle transform)
			: _transform(transform)
		{}

		virtual void update(Vector2) {}
		virtual void draw() const {}
	};
	
	class UIContainer
	{
	protected:
		using ElementPtr = std::shared_ptr<UIElement>;

		std::vector<ElementPtr> _elements;
	public:
		void update(Vector2 mouse)
		{
			std::for_each(_elements.begin(), _elements.end(), [&mouse](UIElement* e) { e->update(mouse); });
		}

		/* @brief add a UI element
		@param args: the parameters necessary to construct the type 
		@return a shared ptr with the added type */
		template<typename _UI, typename... _Args>
		auto add_element(_Args&&... args)
		{
			auto ptr = std::make_shared<_UI>(std::forward<_Args>(args), ...);
			_elements.push_back(ptr);
			return ptr;
		}

		auto begin() { return _elements.begin(); }
		auto end() { return _elements.end(); }
	};

	enum class ButtonState { LeftClicked, RightClicked, Hover, LeftHeld, RightHeld, None };

	/*  */
	class Button 
		: public UIElement
	{
	protected:
		using Callback = std::function<void()>;

		ButtonState _state{ ButtonState::None };
		std::unordered_map<ButtonState, Callback> _callbacks;
		ButtonState _prev_state{ ButtonState::None };

		bool state_has_event(ButtonState state) const
		{
			return _callbacks.find(state) != _callbacks.end();
		}

		void call_state(ButtonState state)
		{
			if (!state_has_event(state)) return;
			_callbacks[state].operator()();
		}

		/* checks if mouse was clicked and returns which button if so */
		ButtonState was_mouse_clicked()
		{
			bool right_clicked = IsMouseButtonPressed(MouseButton::MOUSE_RIGHT_BUTTON);
			bool left_clicked = IsMouseButtonPressed(MouseButton::MOUSE_LEFT_BUTTON);

			if (right_clicked ^ left_clicked) return ButtonState::None; /* no click */
			
			return left_clicked ? ButtonState::LeftClicked : ButtonState::RightClicked;
		}

		ButtonState is_mouse_held()
		{
			bool right_held = 
				(_prev_state == ButtonState::RightClicked || _prev_state == ButtonState::RightHeld) 
				&& IsMouseButtonDown(MouseButton::MOUSE_RIGHT_BUTTON);
			bool left_held = 
				(_prev_state == ButtonState::LeftHeld || _prev_state == ButtonState::LeftHeld)
				&& IsMouseButtonDown(MouseButton::MOUSE_RIGHT_BUTTON);

			if (right_held ^ left_held) return ButtonState::None; /* no click */

			return left_held ? ButtonState::LeftHeld : ButtonState::RightHeld;
		}
	public:
		Button(Rectangle transform)
			: UIElement(transform)
		{}
		
		/* @brief Will bind a callback to a state. Only one callback can be added per event.
		@param state: the state that will trigger the callback cannot be None 
		@param event: the callback to be triggered, is of signature void() */
		void bind(ButtonState state, Callback event)
		{
			_callbacks[state] = event;
		}

		void update(Vector2 mouse) override
		{
			/* check if mouse is on the button */
			if (!helpers::rectangle_v_point(_transform, mouse)) return;
			
			/* check if there was a click */
			ButtonState state = was_mouse_clicked();
			if (state == ButtonState::None)
			{
				state = is_mouse_held();
				if (state == ButtonState::None)
				{
					/* if nothing else, its a hover event */
					state = ButtonState::Hover;
				}
			}

			_prev_state = state;
			call_state(state);
		}

	};
}
