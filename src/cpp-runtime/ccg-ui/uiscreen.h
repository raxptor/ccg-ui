#pragma once

#include <outki/types/ccg-ui/Screen.h>
#include <ccg-ui/uicontext.h>
#include <ccg-ui/uiwidget.h>
#include <kosmos/render/render.h>

namespace ccgui
{
	struct element_handler_set;

	namespace uiscreen
	{
		struct instance;

		struct resolved_texture
		{
			kosmos::render::loaded_texture *texture;
			float u0, v0, u1, v1;
		};

		struct renderinfo
		{
			ccgui::uiscreen::instance *screen;
			ccgui::uicontext *context;
			element_handler_set *handlers;
			float layout_scale;
			float layout_offset_x, layout_offset_y;
			float render_scaling_hint; // how big layout pixels are in real pixels
		};

		instance * create(outki::UIScreen *screen, element_handler_set *handlers);
		void draw(instance *d, uicontext *context, float x0, float y0, float x1, float y1);
		void free(instance *r);

		bool resolve_texture(instance *d, outki::Texture *texture, resolved_texture * out_resolved, float u0, float v0, float u1, float v1);
	}
}
