#include <outki/types/ccg-ui/Elements.h>
#include <putki/liveupdate/liveupdate.h>

#include <kosmos/render/render.h>

#include <ccg-ui/uielement.h>
#include <ccg-ui/uiscreen.h>
#include <ccg-ui/uicontext.h>
#include <ccg-ui/elements/builtins.h>

#include <stdio.h>
#include <iostream>
#include <map>

namespace ccgui
{
	typedef std::map<int, element_handler_def> HandlersMap;
	
	struct element_handler_set
	{
		HandlersMap h;
	};
	
	element_handler_def *get_element_handler(element_handler_set *set, int type)
	{
		HandlersMap::iterator i = set->h.find(type);
		if (i != set->h.end())
			return &i->second;
		return 0;
	}
	
	void set_element_handler(element_handler_set *set, int type, element_handler_def const & def)
	{
		set->h[type] = def;
	}
	
	element_handler_set *create_element_handler_set()
	{
		return new element_handler_set;
	}
	
	void free_element_handler_set(element_handler_set *set)
	{
		delete set;
	}

	namespace uielement
	{
		void draw_fill(uiscreen::renderinfo *rinfo, float x0, float y0, float x1, float y1, outki::UIFill *fill)
		{
			LIVE_UPDATE(&fill);

			if (outki::UIGradientFill *g = fill->exact_cast<outki::UIGradientFill>())
			{
				kosmos::render::gradient_rect(x0, y0, x1, y1, col2int(g->topleft),
				                              col2int(g->topright), col2int(g->bottomleft), col2int(g->bottomright));
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
						
						uiscreen::resolved_texture rt;
						if (uiscreen::resolve_texture(rinfo->screen, g->texture, &rt, us[x], vs[y], us[x+1], vs[y+1]))
						{
							kosmos::render::tex_rect(rt.texture, xs[x], ys[y], xs[x+1], ys[y+1], rt.u0, rt.v0, rt.u1, rt.v1, 0xffffffff);
						}
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
