#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/ccg-renderer.h>
#include <ccg-ui/uielement.h>
#include <ccg-ui/uiscreen.h>
#include <ccg-ui/uicontext.h>
#include <putki/liveupdate/liveupdate.h>

#include <stdio.h>

namespace ccgui
{
	namespace uielement
	{
		void draw_fill(uiscreen::renderinfo *rinfo, float x0, float y0, float x1, float y1, outki::UIFill *fill)
		{
			if (outki::UIGradientFill *g = fill->exact_cast<outki::UIGradientFill>())
			{
				rinfo->backend->gradient_rect(x0, y0, x1, y1, ccgui::col2int(&g->topleft),
				ccgui::col2int(&g->topright), ccgui::col2int(&g->bottomleft), ccgui::col2int(&g->bottomright));
			}
		}
		
		bool hittest(uiscreen::renderinfo *rinfo, float x, float y, float x0, float y0, float x1, float y1)
		{
			return x >= x0 && y >= y0 && x < x1 && y < y1;
		}
		
		bool mousehittest(uiscreen::renderinfo *rinfo, float x0, float y0, float x1, float y1)
		{
			return hittest(rinfo, rinfo->context->input.mouse->x, rinfo->context->input.mouse->y,
			               x0, y0, x1, y1);
		}
		
		bool is_mouseover(uicontext *context, element_id elId)
		{
			return context->mouseover == elId;
		}
		
		bool is_mousepressed(uicontext *context, element_id elId)
		{
			return context->mousedown == elId;
		}
		
		void button_logic(uiscreen::renderinfo *rinfo, element_id elId, float x0, float y0, float x1, float y1)
		{
			if (mousehittest(rinfo, x0, y0, x1, y1))
			{
				// check if mouse was released.
				rinfo->context->mouseover = elId;
			}
			else if (rinfo->context->mouseover == elId)
			{
				// if button released
				if (rinfo->context->mousedown != elId)
					rinfo->context->mouseover = 0;
			}
		}
	}
}
