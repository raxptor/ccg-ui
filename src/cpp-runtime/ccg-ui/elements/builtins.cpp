#include <ccg-ui/uielement.h>
#include <ccg-ui/uiscreen.h>
#include <ccg-ui/uiwidget.h>

#include <putki/liveupdate/liveupdate.h>

#include <kosmos/render/render.h>
#include <kosmos/log/log.h>

#include "uifont.h"

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

	void button_draw(uiscreen::renderinfo *rinfo, outki::UIButtonElement *button, uiwidget::element_layout *layout, dummy *tag)
	{
		uielement::button_logic(rinfo, tag, layout->x0, layout->y0, layout->x1, layout->y1);
		
		if (button->Style)
		{
			LIVE_UPDATE(&button->Style);
		
			outki::UIFill *fill = button->Style->Normal;

			if (uielement::is_mousepressed(rinfo->context, tag))
			{
				fill = button->Style->Pressed;
			}
			else if (uielement::is_mouseover(rinfo->context, tag))
			{
				fill = button->Style->Highlight;
			}
			
			uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, fill);
		}
	}
	
	void fill_draw(uiscreen::renderinfo *rinfo, outki::UIFillElement *fill, uiwidget::element_layout *layout, dummy *tag)
	{
		LIVE_UPDATE(&fill->fill);
		uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, fill->fill);
	}
	
	void text_draw(uiscreen::renderinfo *rinfo, outki::UITextElement *text, uiwidget::element_layout *layout, dummy *tag)
	{
		uifont::data *inst = uifont::create(text->font);
		uifont::layout_data *ld = uifont::layout_make(inst, text->Text, text->pixelSize, -1, 1.0f);
		uifont::layout_draw(ld, layout->x0, layout->y0);
		uifont::layout_free(ld);
		uifont::free(inst);
	}

	void add_builtin_element_handlers(element_handler_set *target)
	{
		set_element_handler<outki::UIBitmapElement, dummy>(target, 0, 0, 0, bitmap_draw, 0);
		set_element_handler<outki::UIButtonElement, dummy>(target, 0, 0, 0, button_draw, 0);
		set_element_handler<outki::UIFillElement, dummy>(target, 0, 0, 0, fill_draw, 0);
		set_element_handler<outki::UITextElement, dummy>(target, 0, 0, 0, text_draw, 0);
	}
}