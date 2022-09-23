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

	/* @return Position where text would be centered inside of a rectangle */
	inline Vector2 center_text_rect(const std::string& str, Rectangle rec, float fontsize, Font font = GetFontDefault())
	{
		const auto size = MeasureTextEx(font, str.c_str(), fontsize, fontsize/10.f);
		return Vector2
		{
			(rec.x + (rec.width / 2.f)) - (size.x / 2.f),
			(rec.y + (rec.height / 2.f)) - (size.y / 2.f)
		};
	}
}

/* should move this to the helpers*/
namespace suite::ui::drawing
{
	/* Custom version of DrawTextRecEx which returns the height required to fit the text but does NOT draw anything 
	incompatible with newer versions of Raylib, as of now version raylib 3.7.0 (26 April 2021) is used */
	float measure_text_height(Font font, const char* text, Rectangle rec, float fontSize, float spacing, bool wordWrap, Color tint, int selectStart = 0, int selectLength = 0, Color selectTint = WHITE, Color selectBackTint = WHITE);
}

namespace suite::ui
{
	namespace
	{
		// static int mouse_setting = MouseCursor::MOUSE_CURSOR_DEFAULT;
		// static void set_mouse_if_default(int mouse)
		// {
		// 	if (mouse_setting != MouseCursor::MOUSE_CURSOR_DEFAULT) return;
		// 	mouse_setting = mouse;
		// }
		// static void set_mouse(int mouse)
		// {
		// 	mouse_setting = mouse;
		// }
	}

	class UIElement
	{
	protected:
		Rectangle _transform;
		bool _hidden{ false };

		virtual void on_draw() const {}
	public:
		UIElement(Rectangle transform)
			: _transform(transform)
		{}

		void hide() { _hidden = true; }
		bool hidden() const { return _hidden; }
		void reveal() { _hidden = false; }

		virtual void update(Vector2) {}

		void draw() const 
		{
			if (_hidden) return;
			on_draw();
		}
	};
	
	class UIContainer
	{
	protected:
		using ElementPtr = std::shared_ptr<UIElement>;

		std::vector<ElementPtr> _elements;

		bool _hide{ false };
	public:
		void update(Vector2 mouse)
		{
			if (_hide) return;
			std::for_each(_elements.begin(), _elements.end(), [&mouse](auto e) { e->update(mouse); });
		}

		void draw() const
		{
			if (_hide) return;
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

		void hide() { _hide = true; }
		void show() { _hide = false; }
	};

	/* TODO: make a global Clickable object that has these states and automatically assigns them to a state */
	enum class ButtonState { LeftClicked, RightClicked, Hover, LeftHeld, RightHeld, WasHover, None };

	/* interface class that adds user interaction to UIElements, make sure to call member function 'handle_state'
	uses the _transform member from base class UIElement to determine interactions */
	class IClickable
		: public UIElement
	{
	private:
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
				(_state == ButtonState::RightClicked || _state == ButtonState::RightHeld)
				&& IsMouseButtonDown(MouseButton::MOUSE_RIGHT_BUTTON);
			bool left_held =
				(_state == ButtonState::LeftClicked || _state == ButtonState::LeftHeld)
				&& IsMouseButtonDown(MouseButton::MOUSE_LEFT_BUTTON);

			if (!(right_held ^ left_held)) return ButtonState::None; /* no click */

			return left_held ? ButtonState::LeftHeld : ButtonState::RightHeld;
		}
	protected:
		ButtonState _state{ ButtonState::None };

		void handle_state(Vector2 mouse)
		{
			/* check if mouse is on item */
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
			else
			{ /* mouse is not on button but if it was then it has just left */
				if (!no_state() && !washover())
					state = ButtonState::WasHover;
			}

			_state = state;
		}
	public:
		using UIElement::UIElement;

		/* handy queries */
		bool no_state() const { return _state == ButtonState::None; }
		bool hover() const { return _state == ButtonState::Hover; }
		bool washover() const { return _state == ButtonState::WasHover; }
		bool leftclicked() const { return _state == ButtonState::LeftClicked; }
	};

	/*  */
	class Button 
		: public IClickable
	{
	protected:
		using Callback = std::function<void()>;

		std::unordered_map<ButtonState, Callback> _callbacks;
		std::string _button_text{}; /* text on button */
		float _fontsize{};
		Vector2 _text_position{};

		bool state_has_event(ButtonState state) const
		{
			return _callbacks.find(state) != _callbacks.end();
		}

		void call_state(ButtonState state)
		{
			if (!state_has_event(state)) return;
			_callbacks[state].operator()();
		}
	public:
		Button(Rectangle transform)
			: IClickable(transform)
		{}
		
		Button(Rectangle transform, float fontsize, std::string button_text)
			: IClickable(transform)
			, _button_text(button_text)
			, _fontsize(fontsize)
			, _text_position(helpers::center_text_rect(_button_text, _transform, _fontsize))
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
			handle_state(mouse);
			
			if (no_state()) return;

			if (hover())
			{
				SetMouseCursor(MouseCursor::MOUSE_CURSOR_POINTING_HAND);
			}
			else if (washover())
			{
				SetMouseCursor(MouseCursor::MOUSE_CURSOR_DEFAULT);
			}

			call_state(_state);
		}

		void on_draw() const override
		{
			DrawRectangleLinesEx(_transform, 3, hover() ? GRAY : BLACK);
			
			if (!has_text()) return;

			DrawTextEx(GetFontDefault(), _button_text.c_str(), _text_position, _fontsize, _fontsize / 10.f, BLACK);
		}

		bool has_text() const { return _button_text.size() > 0; }
	};

	/* non growing input box */
	class StaticInputBox : 
		public IClickable
	{
	protected:
		Font _font{};
		std::string _buffer{};
		std::string _ghost_text{}; /* text that appears when input is empty */
		float _fontsize{};
		bool _full{ false };
		bool _focussed{ false };

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
					_buffer.push_back(key);
					if (validate_text_size())
					{
						pop_back();
						break;
					}
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

		/* @brief sets _full to true if text size too large and returns true */
		bool validate_text_size()
		{
			const auto text_size = measure_text(_buffer);
			return _full = (_fontsize + text_size.x >= _transform.width - _fontsize);
		}

		void draw_carat(Vector2 text_size) const
		{
			const Vector2 position = { _fontsize + _transform.x + text_size.x, _transform.y + (_transform.height / 2.f) - (text_size.y / 2.f) };
			DrawRectangleV(position, { _fontsize / 2.f, _fontsize }, BLACK);
		}

		void draw_text(const std::string& text, Vector2 text_size, Color color = BLACK) const
		{
			const Vector2 position = { _fontsize + _transform.x, std::truncf(_transform.y + ((_transform.height / 2.f) - (text_size.y / 2.f))) };
			DrawTextEx(_font, text.c_str(), position, _fontsize, _fontsize / 10.f, color);
		}
	public:
		StaticInputBox(Rectangle transform, float fontsize, Font font = GetFontDefault())
			: IClickable(transform)
			, _fontsize(fontsize)
			, _font(font)
		{}
		
		StaticInputBox(Rectangle transform, float fontsize, std::string ghost_text, Font font = GetFontDefault())
			: IClickable(transform)
			, _fontsize(fontsize)
			, _font(font)
			, _ghost_text(ghost_text)
		{}

		void update(Vector2 m) override
		{
			handle_state(m);

			/* if clicked anywhere else turn off focus, else turn it on */
			if (IsMouseButtonPressed(MouseButton::MOUSE_LEFT_BUTTON))
				_focussed = leftclicked();

			if (focussed())
			{
				handle_key_input();
			}

			if (hover())
			{
				SetMouseCursor(MOUSE_CURSOR_IBEAM);
				
			}
			else if (washover())
			{
				SetMouseCursor(MOUSE_CURSOR_DEFAULT);
			}
		}

		void on_draw() const override
		{
			DrawRectangleLinesEx(_transform, 4, hover() ? GRAY : BLACK);
			const Vector2 text_size = empty() ? Vector2{ 0.f, _fontsize } : measure_text(_buffer);
			
			if (!empty())
			{
				draw_text(_buffer, text_size);
			}
			else if (has_ghost_text() && !focussed())
			{
				draw_text(_ghost_text, measure_text(_ghost_text), GRAY);
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
		bool focussed() const { return _focussed; }
		bool has_ghost_text() const { return _ghost_text.size() > 0; }
		Vector2 measure_text(const std::string& text) const { return MeasureTextEx(_font, text.c_str(), _fontsize, _fontsize / 10.f); }

		std::string_view buffer() const { return _buffer; }
		auto copy_buffer() const { return _buffer; }
	};

	/* Small box that can be filled with texts and grows with it */
	class DynamicTextBox
		: public UIElement
	{
	protected:
		Font _font;
		std::string _text;
		float _fontsize{};

		Rectangle get_margined_rec() const
		{
			Rectangle margin_transform{ _transform };
			margin_transform.x += _fontsize;
			margin_transform.y += _fontsize;
			margin_transform.width -= _fontsize * 2.f;
			margin_transform.height -= _fontsize * 2.f;
			return margin_transform;
		}

		/* call whenever member transform gets changed, it sets the height of member _transform */
		void size_box_to_text()
		{
			if (_text.size() < 1) return;

			_transform.height = drawing::measure_text_height(_font, _text.c_str(), get_margined_rec(), _fontsize, _fontsize / 10.f, true, BLACK);
			_transform.height += _fontsize * 2.f; /* add margins */
		}
	public:
		/* NOTE: the .height member of transform gets ignored */
		DynamicTextBox(Rectangle transform, float fontsize, std::string text, Font font = GetFontDefault())
			: UIElement(transform)
			, _text(text)
			, _fontsize(fontsize)
			, _font(font)
		{
			size_box_to_text();
		}

		void text(std::string text)
		{
			_text = text;
			size_box_to_text();
		}

		void on_draw() const override
		{
			DrawRectangleLinesEx(_transform, 4, BLACK);
			Rectangle margin_transform = get_margined_rec();
			DrawTextRecEx(_font, _text.c_str(), margin_transform, _fontsize, _fontsize / 10.f, true, BLACK, 0, 0, WHITE, WHITE);
		}
	};
}
