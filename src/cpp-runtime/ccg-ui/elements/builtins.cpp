#include <ccg-ui/uielement.h>
#include <ccg-ui/uiscreen.h>
#include <ccg-ui/uiwidget.h>

#include <kosmos/render/render.h>
#include <kosmos/log/log.h>

namespace ccgui
{
	struct element_handler_set;
	struct dummy {};
	
	void bitmap_draw(uiscreen::renderinfo *rinfo, outki::UIBitmapElement *element, uiwidget::element_layout *layout, dummy *data)
	{
		uiscreen::resolved_texture tex;
		if (uiscreen::resolve_texture(rinfo->screen, element->texture, &tex, 0, 0, 1, 1))
		{
			kosmos::render::tex_rect(tex.texture,
				layout->x0, layout->y0, layout->x1, layout->y1,
				tex.u0, tex.v0, tex.u1, tex.v1, 0xffffffff);
		}
	}
	
	void bitmap_done(dummy *nothing)
	{
		KOSMOS_DEBUG("Destroying bitmap")
	}
	
	namespace
	{
	}

	void add_builtin_element_handlers(element_handler_set *target)
	{
		set_element_handler<outki::UIBitmapElement, dummy>(target, 0, 0, 0, bitmap_draw, 0);
	}
}