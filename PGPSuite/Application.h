#pragma once

#include <raylib.h>

#include "UIElements.h"

namespace suite
{
	class Application
	{
	protected:
		ui::UIContainer _ui;

		virtual void on_input() {}
		virtual void on_update() {}
		virtual void on_draw() const {}
	public:
		Application() = default;
		
		/* handling user input */
		virtual void input()
		{
			const auto m = GetMousePosition();
			_ui.update(m);
			
			on_input();
		}

		/* updating UI elements */
		void update()
		{
			on_update();
		}

		virtual void draw() const
		{
			on_draw();
		}
	};
}
