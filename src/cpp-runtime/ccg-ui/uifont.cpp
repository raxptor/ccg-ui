#include "uifont.h"

#include <ccg-ui/util/utf8.h>
#include <putki/log/log.h>
#include <putki/assert/assert.h>

#include <vector>


namespace ccgui
{
	namespace font
	{
		struct data
		{
			outki::Font *font_data;
		};

		struct layout_glyph
		{
			float x0, y0, x1, y1;
			unsigned int font_glyph_idx;
		};

		struct layout_data
		{
			data *font;
			unsigned int glyphs_size;
			layout_glyph *glyphs;
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
				layout->font = font;
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
						PTK_WARNING("String contains more than max number of glyphs " << MAX_GLYPHS)
						break;
					}
					glyph_id[glyphs++] = (int) codepoint;
				}
			}

			if (state != UTF8_ACCEPT)
				PTK_WARNING("String contains invalid utf8");

			layout_data *layout = new layout_data();
			layout->font = font;
			layout->glyphs_size = glyphs;
			layout->glyphs = new layout_glyph[glyphs];

			return layout;
		}

		void layout_free(layout_data *layout)
		{
			delete layout;
		}
	}
}
