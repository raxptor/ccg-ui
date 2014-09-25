
#include <putki/liveupdate/liveupdate.h>

#include <kosmos/render/render.h>
#include <kosmos/log/log.h>

#include "ccg-ui/uielement.h"
#include "ccg-ui/uiscreen.h"
#include "ccg-ui/uiwidget.h"
#include "ccg-ui/uifont.h"

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

	struct button_data
	{
		button_data() : fill(0), label_layout(0), scale(-1) { }
		outki::UIFill *fill;
		uifont::layout_data *label_layout;
		float scale;
	};


	void button_layout(uiscreen::renderinfo *rinfo, outki::UIButtonElement *button, uiwidget::element_layout *layout, button_data *data)
	{
		const float scale = rinfo->layout_scale * rinfo->render_scaling_hint;
		if (data->scale != scale)
		{
			data->scale = scale;
			if (data->label_layout)
			{
				uifont::layout_free(data->label_layout);
				data->label_layout = 0;
			}
		}
	}

	void button_update(uiscreen::renderinfo *rinfo, outki::UIButtonElement *button, uiwidget::element_layout *layout, button_data *data)
	{
		uielement::button_logic(rinfo, data, layout->x0, layout->y0, layout->x1, layout->y1);

		if (button->Style)
		{
			LIVE_UPDATE(&button->Style);

			outki::Font *fnt = 0;
			if (button->Style->FontStyle && button->Style->FontStyle->Font)
			{
				LIVE_UPDATE(&button->Style->FontStyle);

				fnt = button->Style->FontStyle->Font;
				if (!data->label_layout)
				{
					data->label_layout = uifont::layout_make(fnt, "Baberiba", rinfo->layout_scale * button->Style->FontStyle->PixelSize, -1, rinfo->render_scaling_hint);
				}
			}

			outki::UIFill *fill = button->Style->Normal;

			if (uielement::is_mousepressed(rinfo->context, data))
			{
				fill = button->Style->Pressed;
			}
			else if (uielement::is_mouseover(rinfo->context, data))
			{
				fill = button->Style->Highlight;
			}

			data->fill = fill;
		}
		else
		{
			data->fill = 0;
		}
	}

	void button_draw(uiscreen::renderinfo *rinfo, outki::UIButtonElement *button, uiwidget::element_layout *layout, button_data *data)
	{
		if (data->fill)
		{
			uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, data->fill);
		}
		if (data->label_layout)
		{
			uifont::layout_draw_align(data->label_layout, layout->x0, layout->y0, layout->x1, layout->y1, outki::UIVerticalAlignment_Center, outki::UIHorizontalAlignment_Center, 0xffffffff);
		}
	}

	void fill_draw(uiscreen::renderinfo *rinfo, outki::UIFillElement *fill, uiwidget::element_layout *layout, dummy *tag)
	{
		LIVE_UPDATE(&fill->fill);
		uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, fill->fill);
	}
	
	void text_draw(uiscreen::renderinfo *rinfo, outki::UITextElement *text, uiwidget::element_layout *layout, dummy *tag)
	{
		LIVE_UPDATE(&text->font);
		if (!text->font)
			return;
		
		uifont::layout_data *ld = uifont::layout_make(text->font, text->Text, text->pixelSize * rinfo->layout_scale, -1, rinfo->render_scaling_hint);
		if (ld)
		{
			uifont::layout_draw(ld, layout->x0, layout->y0, col2int(text->color));
			uifont::layout_free(ld);
		}
	}

	void add_builtin_element_handlers(element_handler_set *target)
	{
		set_element_handler<outki::UIBitmapElement, dummy>(target, 0, 0, 0, bitmap_draw, 0);
		set_element_handler<outki::UIButtonElement, button_data>(target, 0, button_layout, button_update, button_draw, 0);
		set_element_handler<outki::UIFillElement, dummy>(target, 0, 0, 0, fill_draw, 0);
		set_element_handler<outki::UITextElement, dummy>(target, 0, 0, 0, text_draw, 0);
	}
}