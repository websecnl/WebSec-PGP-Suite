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
		
		/* call when opengl context is loaded */
		virtual void start() {}

		/* handling user input */
		void input()
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

		void draw() const
		{
			on_draw();
			_ui.draw();
		}
	};

	class ApplicationRunner
	{
	protected:
		Application* _app;
	public:
		ApplicationRunner(Application* app)
			: _app(app)
		{}

		void run()
		{
			constexpr int width = 850;
			constexpr int height = 650;

			InitWindow(850, 650, "Window");

			_app->start();

			while (!WindowShouldClose())
			{
				_app->input();
				_app->update();

				BeginDrawing();
				ClearBackground(WHITE);
				
				_app->draw();

				EndDrawing();
			}

			CloseWindow();
		}
	};
}
