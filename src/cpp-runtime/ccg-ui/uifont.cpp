#include "uifont.h"

#include <ccg-ui/util/utf8.h>
#include <putki/assert/assert.h>

#include <kosmos/log/log.h>
#include <kosmos/render/render.h>

#include <vector>


namespace ccgui
{
	namespace uifont
	{
		struct data
		{
			outki::Font *font_data;
		};

		struct layout_glyph
		{
			float x0, y0, x1, y1;
			outki::FontGlyph *glyph;
		};

		struct layout_data
		{
			unsigned int glyphs_size;
			layout_glyph *glyphs;
			outki::FontOutput *fontdata;
		};

		data* create(outki::Font *font)
		{
			PTK_FATAL_ASSERT(font)

			data *d = new data();
			d->font_data = font;
			return d;
		}

		void free(data *d)
		{
			delete d;
		}

		layout_data *layout_make(data *font, const char *text, float pixel_size, int max_width, float rendering_scale_hint)
		{
			if (!text || !text[0])
				return 0;

			PTK_FATAL_ASSERT(font)
			PTK_FATAL_ASSERT(text)

			outki::Font *source_font = font->font_data;

			PTK_FATAL_ASSERT(source_font)
			PTK_ASSERT(source_font->Outputs_size)

			int best_index = -1;
			float diff = 0.0f;

			// Find the best matching font, taking into account info about rendering scale.
			// If things are rendered with twice the size, let's squeeze in more texels there.
			const float actual_pixel_size = rendering_scale_hint * pixel_size;

			for (int i=0;i!=source_font->Outputs_size;i++)
			{
				outki::FontOutput *fo = &source_font->Outputs[i];
				PTK_FATAL_ASSERT(fo)

				float d = actual_pixel_size - fo->PixelSize;
				if (d < 0)
				{
					d = -d;
				}

				if (!i || d < diff)
				{
					diff = d;
					best_index = i;
				}
			}

			if (best_index < 0)
			{
				layout_data *layout = new layout_data();
				layout->glyphs_size = 0;
				layout->glyphs = 0;
				layout->fontdata = 0;
				return layout;
			}

			const int MAX_GLYPHS = 4096;
			int glyph_id[MAX_GLYPHS];
			int glyphs = 0;

			// utf8 decode
			const uint8_t *ptr = (uint8_t *)text;
			uint32_t state = UTF8_ACCEPT, codepoint;
			for (; *ptr; ++ptr)
			{
				if (!decode(&state, &codepoint, *ptr))
				{
					if (glyphs == MAX_GLYPHS)
					{
						KOSMOS_WARNING("String contains more than max number of glyphs " << MAX_GLYPHS)
						break;
					}
					glyph_id[glyphs++] = (int) codepoint;
				}
			}

			if (state != UTF8_ACCEPT)
				KOSMOS_WARNING("String contains invalid utf8");

			outki::FontOutput *fontdata = &source_font->Outputs[best_index];

			layout_data *layout = new layout_data();
			layout->fontdata = fontdata;
			layout->glyphs_size = glyphs;
			layout->glyphs = new layout_glyph[glyphs];
			
			for (int i=0;i!=glyphs;i++)
			{
				outki::FontGlyph *gldata = 0;
				layout_glyph *current = &layout->glyphs[i];
				
				for (int j=0;j!=fontdata->Glyphs_size;j++)
				{
					if (fontdata->Glyphs[j].glyph == glyph_id[i])
					{
						gldata = &fontdata->Glyphs[j];
						break;
					}
				}
				
				current->glyph = gldata;
				
				if (!gldata)
					continue;

				current->x0 = i * 20;
				current->y0 = 0;
				current->x1 = i * 20 + gldata->pixelWidth;
				current->y1 = gldata->pixelHeight;
			}
			
			return layout;
		}

		void layout_draw(layout_data *layout, float x, float y)
		{
			kosmos::render::loaded_texture *tex = kosmos::render::load_texture(layout->fontdata->OutputTexture);
			for (int i=0;i<layout->glyphs_size;i++)
			{
				layout_glyph *cur = &layout->glyphs[i];
				if (!cur->glyph)
					continue;
					
				outki::FontGlyph *glyph = cur->glyph;
		
				kosmos::render::tex_rect(tex,
					x + cur->x0,
					y + cur->y0,
					x + cur->x1,
					y + cur->y1,
					glyph->u0,
					glyph->v0,
					glyph->u1,
					glyph->v1,
					0xffffffff
				);
			}
		/*
			kosmos::render::tex_rect(tex,
					0, 0, 256, 256, 0, 0, 1, 1, 0xffffffff);
		*/
			kosmos::render::unload_texture(tex);
		}

		void layout_free(layout_data *layout)
		{
			delete layout;
		}
	}
}
