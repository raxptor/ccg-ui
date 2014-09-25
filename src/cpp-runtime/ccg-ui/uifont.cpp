#include "uifont.h"

#include <ccg-ui/util/utf8.h>

#include <outki/types/ccg-ui/Font.h>
#include <outki/types/ccg-ui/Elements.h>

#include <putki/assert/assert.h>

#include <kosmos/log/log.h>
#include <kosmos/render/render.h>

#include <vector>


namespace ccgui
{
	namespace uifont
	{
		layout_data *layout_make(outki::Font *font, const char *text, float pixel_size, int max_width, float rendering_scale_hint)
		{
			if (!text || !text[0])
				return 0;

			PTK_FATAL_ASSERT(font)
			PTK_FATAL_ASSERT(font->Outputs_size)
			PTK_FATAL_ASSERT(text)

			int best_index = -1;
			float diff = 0.0f;

			// Find the best matching font, taking into account info about rendering scale.
			// If things are rendered with twice the size, let's squeeze in more texels there.
			const float actual_pixel_size = rendering_scale_hint * pixel_size;
			float scaling = 1.0f;

			for (int i=0;i!=font->Outputs_size;i++)
			{
				const outki::FontOutput *fo = &font->Outputs[i];
				PTK_FATAL_ASSERT(fo)

				float d = actual_pixel_size - fo->PixelSize;
				if (d < 0)
				{
					d = -0.30f * d;
				}

				if (!i || d < diff)
				{
					diff = d;
					best_index = i;
					scaling = pixel_size / fo->PixelSize;
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
			bool word_break[MAX_GLYPHS];
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
					word_break[glyphs] = (codepoint == ' ');
					glyph_id[glyphs++] = (int) codepoint;
				}
			}

			if (state != UTF8_ACCEPT)
				KOSMOS_WARNING("String contains invalid utf8");

			const outki::FontOutput *fontdata = &font->Outputs[best_index];

			layout_data *layout = new layout_data();
			layout->fontdata = fontdata;
			layout->glyphs_size = glyphs;
			layout->glyphs = new layout_glyph[glyphs];

			// lookup and translate, might output less glyphs than allocated
			// if the font does not contain all.
			layout->glyphs_size = 0;
			for (int i=0;i!=glyphs;i++)
			{
				const outki::FontGlyph *gldata = 0;

				for (int j=0;j!=fontdata->Glyphs_size;j++)
				{
					if (fontdata->Glyphs[j].glyph == glyph_id[i])
					{
						const int pos = layout->glyphs_size++;

						// store the re-mapped glyph id in the array
						glyph_id[pos] = j;
						layout->glyphs[pos].glyph = gldata;
						break;
					}
				}
			}

			if (!layout->glyphs_size)
			{
				delete layout;
				return 0;
			}

			layout->face_y0 = - scaling * fontdata->BBoxMaxY / 64.0f;
			layout->face_y1 = - scaling * fontdata->BBoxMinY / 64.0f;

			float pen_break = max_width * fontdata->BBoxMaxY / 64.0f;
			int y_ofs = 0;
			int pen = 0;

			// do actual layouting.
			layout->lines = 0;

			for (int i=0;i!=layout->glyphs_size;i++)
			{
				const outki::FontGlyph *glyph = &fontdata->Glyphs[glyph_id[i]];
				layout_glyph *current = &layout->glyphs[i];

				if (pen > 0)
				{
					int left = glyph_id[i-1];
					int right = glyph_id[i];

					for (int k=0;k!=fontdata->KerningCharL_size;k++)
					{
						if (fontdata->KerningCharL[k] == left && fontdata->KerningCharR[k] == right)
						{
							pen += (int)(scaling * fontdata->KerningOfs[k]);
							break;
						}
					}
				}

				const int x = (float)(scaling * ((pen + glyph->bearingX) / 64.0f));
				const int y = (float)(scaling * (((y_ofs + glyph->bearingY) >> 6)));
				const int w = (float)(scaling * glyph->pixelWidth);
				const int h = (float)(scaling * glyph->pixelHeight);

				current->x0 = x;
				current->y0 = y;
				current->x1 = x + w;
				current->y1 = y + h;
				current->glyph = glyph;

				if (actual_pixel_size < 20.0f && false)
				{
					current->x0 = ((int)(current->x0 * rendering_scale_hint + 0.5f)) / rendering_scale_hint;
					current->y0 = ((int)(current->y0 * rendering_scale_hint + 0.5f)) / rendering_scale_hint;
					current->x1 = ((int)(current->x1 * rendering_scale_hint + 0.5f)) / rendering_scale_hint;
					current->y1 = ((int)(current->y1 * rendering_scale_hint + 0.5f)) / rendering_scale_hint;
				}

				if (!i || current->x1 > layout->maxx) layout->maxx = current->x1;
				if (!i || current->y1 > layout->maxy) layout->maxy = current->y1;
				if (!i || current->x0 < layout->minx) layout->minx = current->x0;
				if (!i || current->y0 < layout->miny) layout->miny = current->y0;

				pen += glyph->advance;

				if (pen_break > 0 && pen > pen_break)
				{
					// do word wrap.
					int k = i;
					while (k > 0)
					{
						if (word_break[k])
						{
							// leave the wordbreak on that line and start next
							i = k;
							pen = 0;
							y_ofs += fontdata->BBoxMaxY - fontdata->BBoxMinY;
							break;
						}
						k--;
					}
					// reset to a new line
					if (pen == 0)
					{
						// relayout from here.
						layout->lines++;
						continue;
					}
				}
			}

			return layout;
		}

		void layout_draw(layout_data *layout, float x, float y, unsigned long color)
		{
			kosmos::render::loaded_texture *tex = kosmos::render::load_texture(layout->fontdata->OutputTexture);
			for (int i=0;i<layout->glyphs_size;i++)
			{
				layout_glyph *cur = &layout->glyphs[i];
				if (!cur->glyph)
					continue;
					
				const outki::FontGlyph *glyph = cur->glyph;
		
				kosmos::render::tex_rect(tex,
					x + cur->x0,
					y + cur->y0,
					x + cur->x1,
					y + cur->y1,
					glyph->u0,
					glyph->v0,
					glyph->u1,
					glyph->v1,
					color
				);
			}

//			kosmos::render::tex_rect(tex,
//					0, 0, 256, 256, 0, 0, 1, 1, 0xffffffff);

			kosmos::render::unload_texture(tex);
		}

		void layout_draw_align(layout_data *layout, float x0, float y0, float x1, float y1, int v, int h, unsigned long color)
		{
			float x, y;

			switch (h)
			{
				case outki::UIHorizontalAlignment_Center:
					x = (x0 + x1 - (layout->maxx - layout->minx)) / 2 - layout->minx;
					break;
				case outki::UIHorizontalAlignment_Right:
					x = x1 - (layout->maxx);
					break;
				default: // left align
					x = x0 - layout->minx;
					break;
			}

			switch (v)
			{
				case outki::UIVerticalAlignment_Top:
					y = y0 - layout->miny;
					break;
				case outki::UIVerticalAlignment_Bottom:
					y = y1 - layout->maxy;
					break;
				default: // center
					{
						float ya = (y0 + y1 - (layout->face_y1 - layout->face_y0)) / 2 - layout->face_y0;
						float yb = (y0 + y1 - (layout->maxy - layout->miny)) / 2 - layout->miny;
						y = (ya + yb) * 0.5f;
						break;
					}
			}

			return layout_draw(layout, x, y, color);
		}

		void layout_free(layout_data *layout)
		{
			delete layout;
		}
	}
}
