#pragma once

#include <raylib.h>
#include <functional>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <string>

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
			std::for_each(_elements.begin(), _elements.end(), [&mouse](auto e) { e->update(mouse); });
		}

		void draw() const
		{
			std::for_each(_elements.begin(), _elements.end(), [](const auto e) { e->draw(); });
		}

		/* @brief add a UI element
		@param args: the parameters necessary to construct the type 
		@return a shared ptr with the added type */
		template<typename _UI, typename... _Args>
		auto add_element(_Args&&... args)
		{
			auto ptr = std::make_shared<_UI>(std::forward<_Args>(args)...);
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

			if (!(right_clicked ^ left_clicked)) return ButtonState::None; /* no click */
			
			return left_clicked ? ButtonState::LeftClicked : ButtonState::RightClicked;
		}

		ButtonState is_mouse_held()
		{
			bool right_held = 
				(_prev_state == ButtonState::RightClicked || _prev_state == ButtonState::RightHeld) 
				&& IsMouseButtonDown(MouseButton::MOUSE_RIGHT_BUTTON);
			bool left_held = 
				(_prev_state == ButtonState::LeftClicked || _prev_state == ButtonState::LeftHeld)
				&& IsMouseButtonDown(MouseButton::MOUSE_RIGHT_BUTTON);

			if (!(right_held ^ left_held)) return ButtonState::None; /* no click */

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
			ButtonState state{ ButtonState::None };
			if (helpers::rectangle_v_point(_transform, mouse))
			{
				/* check if there was a click */
				state = was_mouse_clicked();
				if (state == ButtonState::None)
				{
					state = is_mouse_held();
					if (state == ButtonState::None)
					{
						/* if nothing else, its a hover event */
						state = ButtonState::Hover;
					}
				}
			}
			_prev_state = state;
			call_state(state);
		}

		void draw() const override
		{
			DrawRectangleLinesEx(_transform, 3, hover() ? GRAY : BLACK);
		}

		bool hover() const { return _prev_state == ButtonState::Hover; }
	};

	enum class InputBoxState { None, Hover, Focussed, FocussedHover };

	/* non growing input box */
	class StaticInputBox : 
		public UIElement
	{
	protected:
		Font _font{};
		std::string _buffer{};
		InputBoxState _state{ InputBoxState::None };
		float _fontsize{};
		bool _full{ false };

		void pop_back()
		{
			if (size() > 0)
				_buffer.pop_back();
		}

		void handle_key_input()
		{
			/* load in first keypress on queue if there is, if not it returns 0 */
			int key = GetCharPressed();

			if (!full())
			{
				while (key > 0)
				{
					/* only allowing keys in range 32 ... 125 */
					//if ((key >= 32) && (key <= 125))
					//{
						_buffer.push_back(key);
						if (validate_text_size())
						{
							pop_back();
							break;
						}
					// }
					/* get next key in queue */
					key = GetCharPressed();
				}
			}

			if (IsKeyPressed(KEY_BACKSPACE))
			{
				pop_back();
				_full = false; /* we removed something so it cant be full */
			}
		}

		void set_input_state(Vector2 m)
		{
			bool is_hover = helpers::rectangle_v_point(_transform, m);
			bool clicked = IsMouseButtonPressed(MouseButton::MOUSE_LEFT_BUTTON);
			bool is_clicked = is_hover && clicked;
			/* make sure to redo the hover check otherwise hover stays forever */
			_state = focussed() ? InputBoxState::Focussed : InputBoxState::None;

			if (is_clicked) 
				_state = InputBoxState::Focussed;

			if (is_hover) /* if hover and was focussed set both else just hover */
				_state = _state == InputBoxState::Focussed ? InputBoxState::FocussedHover : InputBoxState::Hover;
			else if (clicked) /* if clicked but not on inputbox, turn off */
				_state = InputBoxState::None;
		}

		/* @brief sets _full to true if text size too large and returns true */
		bool validate_text_size()
		{
			const auto text_size = measure_text();
			return _full = (_fontsize + text_size.x >= _transform.width - _fontsize);
		}

		void draw_carat(Vector2 text_size) const
		{
			const Vector2 position = { _fontsize + _transform.x + text_size.x, _transform.y + (_transform.height / 2.f) - (text_size.y / 2.f) };
			DrawRectangleV(position, { _fontsize / 2.f, _fontsize }, BLACK);
		}

		void draw_text(Vector2 text_size) const
		{
			const Vector2 position = { _fontsize + _transform.x, std::truncf(_transform.y + ((_transform.height / 2.f) - (text_size.y / 2.f))) };
			DrawTextEx(_font, _buffer.c_str(), position, _fontsize, _fontsize / 10.f, BLACK);
		}
	public:
		StaticInputBox(Rectangle transform, float fontsize, Font font = GetFontDefault())
			: UIElement(transform)
			, _fontsize(fontsize)
			, _font(font)
		{}

		void update(Vector2 m) override
		{
			set_input_state(m);

			if (focussed())
			{
				handle_key_input();
			}

			if (hovered())
			{
				SetMouseCursor(MOUSE_CURSOR_IBEAM);
			}
			else
			{
				SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			}
			
		}

		void draw() const override
		{
			DrawRectangleLinesEx(_transform, 4, hovered() ? GRAY : BLACK);
			const Vector2 text_size = empty() ? Vector2{ 0.f, _fontsize } : measure_text();
			
			if (!empty())
			{
				draw_text(text_size);
			}
			
			if (focussed() && !full())
			{
				draw_carat(text_size);
			}
		}

		void clear() { _buffer.clear(); }
		
		size_t size() const { return _buffer.size(); }
		bool empty() const { return size() == 0; }
		bool full() const { return _full; }
		bool focussed() const { return _state == InputBoxState::Focussed || _state == InputBoxState::FocussedHover; }
		bool hovered() const { return _state == InputBoxState::Hover || _state == InputBoxState::FocussedHover; }
		Vector2 measure_text() const { return MeasureTextEx(_font, _buffer.c_str(), _fontsize, _fontsize / 10.f); }

		std::string_view buffer() const { return _buffer; }
		auto copy_buffer() const { return _buffer; }
	};
}
