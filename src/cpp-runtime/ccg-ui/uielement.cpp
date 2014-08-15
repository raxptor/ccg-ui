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
			LIVE_UPDATE(&fill);
			if (outki::UIGradientFill *g = fill->exact_cast<outki::UIGradientFill>())
			{
				rinfo->backend->gradient_rect(x0, y0, x1, y1, ccgui::col2int(&g->topleft),
				                              ccgui::col2int(&g->topright), ccgui::col2int(&g->bottomleft), ccgui::col2int(&g->bottomright));
			}
			else if (outki::UISlice9Fill *g = fill->exact_cast<outki::UISlice9Fill>())
			{
				float _x0 = x0 - g->ExpandLeft;
				float _y0 = y0 - g->ExpandTop;
				float _x1 = x1 + g->ExpandRight;
				float _y1 = y1 + g->ExpandBottom;

				float xs[4] = {_x0, _x0 + g->MarginLeft, _x1 - g->MarignRight, _x1 };
				float ys[4] = {_y0, _y0 + g->MarginLeft, _y1 - g->MarignRight, _y1 };
				float us[4] = {0, g->MarginLeft, g->texture->Width - g->MarignRight, g->texture->Width };
				float vs[4] = {0, g->MarginTop, g->texture->Height - g->MarginBottom, g->texture->Height };

				for (int i=0;i<4;i++)
				{
					us[i] /= g->texture->Width;
					vs[i] /= g->texture->Height;
				}

				for (int y=0;y<3;y++)
				{
					if (ys[y] == ys[y+1])
					{
						continue;
					}

					for (int x=0;x<3;x++)
					{
						if (xs[x] == xs[x+1])
						{
							continue;
						}

						rinfo->backend->tex_rect(g->texture, xs[x], ys[y], xs[x+1], ys[y+1], us[x], vs[y], us[x+1], vs[y+1], 0xffffffff);
					}
				}
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

		bool button_logic(uiscreen::renderinfo *rinfo, element_id elId, float x0, float y0, float x1, float y1)
		{
			bool clicked = false;
			mouse_input *mouse = rinfo->context->input.mouse;

			if (!mouse->primary.isDown || mouse->primary.wentUp)
			{
				if (mousehittest(rinfo, x0, y0, x1, y1))
				{
					rinfo->context->mouseover = elId;
				}
			}

			if (rinfo->context->mouseover == elId)
			{
				if (mouse->primary.isDown || mouse->primary.wentDown)
				{
					rinfo->context->mousedown = elId;
				}
			}

			// released while having mousedown
			if (!mouse->primary.isDown && rinfo->context->mousedown == elId)
			{
				rinfo->context->mousedown = 0;
				if (mousehittest(rinfo, x0, y0, x1, y1))
				{
					clicked = true;
				}
			}

			if (!mouse->primary.isDown && rinfo->context->mouseover == elId)
			{
				if (!mousehittest(rinfo, x0, y0, x1, y1))
				{
					rinfo->context->mouseover = 0;
				}
			}



			if (clicked)
			{
				printf("Clicked button!\n");
			}

			return clicked;
		}
	}
}
