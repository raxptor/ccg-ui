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
			// non-layout scaled layout
			float nsx0, nsy0, nsx1, nsy1;
			// layout-scaled coordinates.
			float x0, y0, x1, y1;
		};

		instance *create(outki::UIWidget *screen, uiscreen::renderinfo *handler_set);
		void layout(instance *d, uiscreen::renderinfo *rinfo, const element_layout *layout);
		void update(instance *d, uiscreen::renderinfo *context);
		void draw(instance *d, uiscreen::renderinfo *rinfo);
		void free(instance *r);
	}
}