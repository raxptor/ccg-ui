#pragma once

namespace ccgui
{
	typedef void* element_id;

	struct mouse_button
	{
		int wentDown, wentUp;
		bool isDown;
	};

	struct mouse_input
	{
		float x, y;
		mouse_button primary;
	};

	struct frame_input
	{
		mouse_input *mouse;
	};

	struct uicontext
	{
		uicontext()
		{
			mouseover = 0;
			mousedown = 0;
			input.mouse = 0;
		}

		frame_input input;

		element_id mouseover;
		element_id mousedown;
	};
}
