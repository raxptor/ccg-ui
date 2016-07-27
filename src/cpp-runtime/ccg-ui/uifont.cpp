#include "uifont.h"

#include <ccg-ui/util/utf8.h>

#include <outki/types/ccg-ui/Font.h>
#include <outki/types/ccg-ui/Elements.h>

#include <putki/assert/assert.h>

#include <kosmos/log/log.h>
#include <kosmos/render/render.h>
#include <kosmos/render/render2d.h>

#include <vector>
#include <math.h>

#include "glyphcache.h"

namespace ccgui
{
	namespace uifont
	{
		namespace
		{
			void unpack_rle_row(unsigned char *rleBuf, unsigned int size, unsigned char *output, unsigned int width)
			{
				int pos = 0;
				unsigned char *input = rleBuf;
				unsigned char *end = rleBuf + size;
				while (input < end)
				{
					if (input[0] & 0x80)
					{
						int count = ((*input++) & 0x7f) + 1;
						unsigned char value = (*input++);
						for (int j=0;j<count;j++)
							output[pos++] = value;
					}
					else
					{
						unsigned char res = (*input++) * 2;
						if (res == 254) res = 255;
						output[pos++] = res;
					}
				}

				if (pos > (int)width)
				{
					KOSMOS_ERROR("unpack_rle_row unpacked too much! " << pos << " but expected " << width)
				}

				while (pos < (int)width)
					output[pos++] = 0;
			}
		}

		layout_data *layout_make(outki::font *font, glyphcache::data *cache, const char *text, float pixel_size, int max_width, float rendering_scale_hint)
		{
			if (!text || !text[0])
				return 0;

			PTK_FATAL_ASSERT(font)
			PTK_FATAL_ASSERT(font->outputs_size)
			PTK_FATAL_ASSERT(text)

			int best_index = -1;
			float diff = 0.0f;

			// Find the best matching font, taking into account info about rendering scale.
			// If things are rendered with twice the size, let's squeeze in more texels there.
			const float actual_pixel_size = rendering_scale_hint * pixel_size;
			float scaling = 1.0f;

			for (int i=0;i!=font->outputs_size;i++)
			{
				const outki::font_output *fo = &font->outputs[i];
				PTK_FATAL_ASSERT(fo)

				float d = actual_pixel_size - fo->pixel_size;
				if (d < 0)
				{
					d = -0.30f * d;
				}

				if (!i || d < diff)
				{
					diff = d;
					best_index = i;
					scaling = pixel_size / fo->pixel_size;
				}
			}

			const bool pixel_snap = (pixel_size < 16);

			if (pixel_snap)
			{
				scaling = 1.0f;
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

			const outki::font_output *fontdata = &font->outputs[best_index];

			layout_data *layout = new layout_data();
			layout->fontdata = fontdata;
			layout->glyphs_size = glyphs;
			layout->glyphs = new layout_glyph[glyphs];

			// lookup and translate, might output less glyphs than allocated
			// if the font does not contain all.

			const outki::font_glyph_pix_data *glyphData[MAX_GLYPHS];

			layout->glyphs_size = 0;
			for (int i=0;i!=glyphs;i++)
			{
				layout_glyph *l = &layout->glyphs[layout->glyphs_size];
				bool gotit = false;

				for (int j=0;j!=fontdata->pix_glyphs_size;j++)
				{
					const outki::font_glyph_pix_data *pg = &fontdata->pix_glyphs[j];
					glyphData[layout->glyphs_size] = pg;

					if (pg->glyph == glyph_id[i])
					{
						void *handle = (void*) pg;
						if (!glyphcache::get(cache, handle, l->u, l->v, &l->tex))
						{
							unsigned char uncomp[65536];
							memset(uncomp, 0x00, sizeof(uncomp));
							for (int h=0;h<pg->pixel_height;h++)
								unpack_rle_row(font->rle_data + pg->rle_data_begin[h], pg->rle_data_length[h], &uncomp[h * pg->pixel_width], pg->pixel_width);

							// insert and retry.
							glyphcache::insert(cache, handle, uncomp, pg->pixel_width, pg->pixel_height);

							//
							gotit = glyphcache::get(cache, handle, l->u, l->v, &l->tex);
						}
						else
						{
							gotit = true;
						}

						break;
					}
				}

				if (gotit)
				{
					layout->glyphs_size++;
				}
			}

			if (!layout->glyphs_size)
			{
				delete layout;
				return 0;
			}

			layout->face_y0 = - scaling * fontdata->b_box_max_y / 64.0f;
			layout->face_y1 = -scaling * fontdata->b_box_min_y / 64.0f;

			float pen_break = max_width * fontdata->b_box_max_y / 64.0f;
			int y_ofs = 0;
			int pen = 0;

			// do actual layouting.
			layout->lines = 0;

			for (int i=0;i!=layout->glyphs_size;i++)
			{
				const outki::font_glyph_pix_data *glyph = glyphData[i];
				layout_glyph *current = &layout->glyphs[i];

				if (pen > 0)
				{
					int left = glyph_id[i-1];
					int right = glyph_id[i];

					for (int k=0;k!=fontdata->kerning_char_l_size;k++)
					{
						if (fontdata->kerning_char_l[k] == left && fontdata->kerning_char_r[k] == right)
						{
							pen += (int)(scaling * fontdata->kerning_ofs[k]);
							break;
						}
					}
				}

				float x = (float)(scaling * ((pen + glyph->bearing_x) / 64.0f));
				float y = (float)(scaling * (((y_ofs + glyph->bearing_y) >> 6)));
				const int w = (float)(scaling * glyph->pixel_width);
				const int h = (float)(scaling * glyph->pixel_height);

				if (pixel_snap)
				{
					x = (int)x;
					y = (int)y;
				}

				current->x0 = x;
				current->y0 = y;
				current->x1 = x + w;
				current->y1 = y + h;

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
							y_ofs += fontdata->b_box_max_y - fontdata->b_box_min_y;
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

			layout->pixel_snapped = pixel_snap;
			return layout;
		}

		void layout_draw(kosmos::render2d::stream *stream, layout_data *layout, float x, float y, unsigned long color)
		{
			if (layout->pixel_snapped)
			{
				x = floorf(x);
				y = floorf(y);
			}

			for (int i=0;i<layout->glyphs_size;i++)
			{
				layout_glyph *cur = &layout->glyphs[i];
				if (!cur->tex)
					continue;

/*				if (i == 0)
				{
					kosmos::render2d::tex_rect(stream, cur->tex, 0, 0, 256, 256, 0, 0, 1 ,1, color);
				}
*/
				kosmos::render2d::tex_rect(stream, cur->tex,
					x + cur->x0,
					y + cur->y0,
					x + cur->x1,
					y + cur->y1,
					cur->u[0],
					cur->v[0],
				       cur->u[1],
					cur->v[1],
				       color
				);
			}
		}

		void layout_draw_align(kosmos::render2d::stream *stream, layout_data *layout, float x0, float y0, float x1, float y1, int v, int h, unsigned long color)
		{
			float x, y;

			switch (h)
			{
				case outki::UI_HORIZONTAL_ALIGNMENT_CENTER:
					x = (x0 + x1 - (layout->maxx - layout->minx)) / 2 - layout->minx;
					break;
				case outki::UI_HORIZONTAL_ALIGNMENT_RIGHT:
					x = x1 - (layout->maxx);
					break;
				default: // left align
					x = x0 - layout->minx;
					break;
			}

			switch (v)
			{
				case outki::UI_VERTICAL_ALIGNMENT_TOP:
					y = y0 - layout->miny;
					break;
				case outki::UI_VERTICAL_ALIGNMENT_BOTTOM:
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

			return layout_draw(stream, layout, x, y, color);
		}

		void layout_free(layout_data *layout)
		{
			delete layout;
		}
	}
}
