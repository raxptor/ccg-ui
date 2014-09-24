#pragma once

#include <outki/types/ccg-ui/Screen.h>
#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/uiscreen.h>
#include <ccg-ui/uielement.h>

namespace ccgui
{
	namespace uiscreen
	{
		struct renderinfo;
	}

	namespace uiwidget
	{
		struct instance;		
		struct element_layout
		{
			float x0, y0, x1, y1;
		};

		instance *create(outki::UIWidget *screen, uiscreen::renderinfo *handler_set);
		void layout(instance *d, uiscreen::renderinfo *rinfo, float x0, float y0, float x1, float y1);
		void update(instance *d, uiscreen::renderinfo *context);
		void draw(instance *d, uiscreen::renderinfo *rinfo);
		void free(instance *r);
	}
}