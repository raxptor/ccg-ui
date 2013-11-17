#pragma once

#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/ccg-renderer.h>
#include <ccg-ui/uicontext.h>
#include <ccg-ui/uiscreen.h>

namespace ccgui
{
	namespace uiscreen { struct instance; }

	namespace uielement
	{
		bool hittest(uiscreen::renderinfo *rinfo, float x, float y, float x0, float y0, float x1, float y1);

		bool is_mouseover(uicontext *context, element_id elId);
		bool is_mousepressed(uicontext *context, element_id elId);
		
		bool button_logic(uiscreen::renderinfo *rinfo, element_id elId, float x0, float y0, float x1, float y1);

		// generic components
		void draw_fill(uiscreen::renderinfo *rinfo, float x0, float y0, float x1, float y1, outki::UIFill *fill);
	}
}
