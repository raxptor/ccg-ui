#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/build-db.h>
#include <putki/builder/log.h>

#include <iostream>
#include <sstream>
#include <vector>

#include <inki/types/ccg-ui/Font.h>
#include <inki/types/kosmos/Texture.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <sstream>

#include <kosmos-builder-utils/binpacker/maxrects_binpack.h>
#include <kosmos-builder-utils/pngutil.h>

#include "rowcache.h"

struct TmpGlyphInfo
{
	int w, h;
	int bearingX, bearingY;
	int advance;
	unsigned char *data;
};

namespace 
{
	void make_outline(unsigned int *dest, unsigned int *source, int width, int height)
	{
		//
		for (int y=0;y<height;y++)
		{
			for (int x=0;x<width;x++)
			{
				int max = 0;
				for (int v=-1;v<2;v++)
				{
					const unsigned int Y = y + v;
					if (!(Y < height))
						continue;
					for (int u=-1;u<2;u++)
					{
						static const int mul[9] = {180,256,180,256,256,256,180,256,180};
						const unsigned int X = x + u;
						if (!(X < width))
							continue;
						unsigned int val = source[Y * width + X] >> 24;
						val = (val * mul[(1+v)*3+(1+u)]) >> 8;
						if (val > max)
							max = val;
					}	
				}
				dest[y*width+x] = max << 24;
			}
		}
	}
	
	void blend_outline(unsigned int *dest, unsigned int *outline, unsigned int *original, int width, int height)
	{
		for (int y=0;y<height;y++)
		{
			for (int x=0;x<width;x++)
			{
				unsigned int idx = y * width + x;
				unsigned int white = (original[idx] >> 24) & 0xff; 
				dest[idx] = (outline[idx] & 0xff000000) | (white * 0x10101);
			}
		}
	}

	bool build_font(const putki::builder::build_info* info)
	{
		inki::font *font = (inki::font *) info->object;

		RECORD_INFO(info->record, "Source is " << font->source)

		if (font->latin1)
		{
			for (int i='a';i<='z';i++)
				font->characters.push_back(i);
			for (int i='A';i<='Z';i++)
				font->characters.push_back(i);
			for (int i='0';i<='9';i++)
				font->characters.push_back(i);

			const char *special = "!()#?:/\\<>[] .,";
			for (int i=0;i<strlen(special);i++)
				font->characters.push_back(special[i]);
		}

		row_cache cache;

		putki::builder::resource res;
		if (!putki::builder::fetch_resource(info, font->source.c_str(), &res))
		{
			RECORD_WARNING(info->record, "Could not load font at [" << font->source << "]!")
			return false;
		}

		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			RECORD_WARNING(info->record, "Could not initialize freetype");
			putki::builder::free_resource(&res);
			return false;
		}

		FT_Face face;
		if (FT_New_Memory_Face(ft, (const FT_Byte *)res.data, (FT_Long)res.size, 0, &face))
		{
			RECORD_WARNING(info->record, "Could not load font face");
			putki::builder::free_resource(&res);
			FT_Done_FreeType(ft);
			return false;
		}

		int totalGlyphs = 0;
		int totalGlyphPixelData = 0;

		for (unsigned int sz=0;sz<font->pixel_sizes.size();sz++)
		{
			if (FT_Set_Pixel_Sizes(face, 0, font->pixel_sizes[sz]))
			{
				RECORD_WARNING(info->record, "Could not set char or pixel size.");
				goto cleanup;
			}

			std::vector<rbp::InputRect> packs;
			std::vector<TmpGlyphInfo> glyphs;

			inki::font_output up;
			up.pixel_size = font->pixel_sizes[sz];

			up.b_box_min_y = 1000000;
			up.b_box_max_y = -100000;

			int border = 2 + font->outline_width;

			for (unsigned int i=0;i<font->characters.size();i++)
			{
				int idx = FT_Get_Char_Index(face, font->characters[i]);
				if (FT_Load_Glyph(face, idx, FT_LOAD_NO_BITMAP))
				{
					RECORD_WARNING(info->record, "Could not load glyph face.");
					continue;
				}

				if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
				{
					RECORD_WARNING(info->record, "Could not render glyph.");
					continue;
				}

				FT_Bitmap *bmp = &face->glyph->bitmap;

				TmpGlyphInfo g;
				g.data = new unsigned char[bmp->width * bmp->rows];
				g.w = bmp->width;
				g.h = bmp->rows;
				g.bearingX = face->glyph->metrics.horiBearingX;
				g.bearingY = face->glyph->metrics.horiBearingY;
				g.advance = face->glyph->metrics.horiAdvance;
				glyphs.push_back(g);

				const int y0 = g.bearingY - 64 * g.h;
				const int y1 = g.bearingY;
				if (y0 < up.b_box_min_y)
				{
					up.b_box_min_y = y0;
				}
				if (y1 > up.b_box_max_y)
				{
					up.b_box_max_y = y1;
				}

				for (unsigned int j=0;j<font->characters.size();j++)
				{
					FT_Vector kerning;
					if (!FT_Get_Kerning(face, font->characters[i], font->characters[j], FT_KERNING_DEFAULT, &kerning))
					{
						if (kerning.x != 0)
						{
							up.kerning_char_l.push_back(font->characters[i]);
							up.kerning_char_r.push_back(font->characters[j]);
							up.kerning_ofs.push_back(kerning.x);
						}
					}
				}

				for (int y=0;y<bmp->rows;y++)
				{
					for (int x=0;x<bmp->width;x++)
					{
						g.data[y * bmp->width + x] = bmp->buffer[y * bmp->width + x];
					}
				}

				rbp::InputRect next;
				next.id = i;
				next.width = bmp->width + 2 * border;
				next.height = bmp->rows + 2 * border;
				packs.push_back(next);
			}

			std::vector<rbp::Rect> packedRects;

			int out_width = 16;
			int out_height = 16;

			while (true)
			{
				packedRects.clear();

				rbp::MaxRectsBinPack pack(out_width, out_height);
				std::vector< rbp::InputRect > tmpCopy = packs;
				pack.Insert(tmpCopy, packedRects, rbp::MaxRectsBinPack::RectBottomLeftRule);

				if (packedRects.size() == packs.size())
				{
					break;
				}
				else
				{
					if (out_height > out_width)
					{
						out_width *= 2;
					}
					else
					{
						out_height *= 2;
					}
				}
			}

			unsigned int * outBmp = 0;

			if (font->texture_configuration)
			{
				outBmp = new unsigned int[out_width * out_height];
				for (int y=0;y<out_height;y++)
				{
					for (int x=0;x<out_width;x++)
					{
						outBmp[y*out_width+x] = 0x0;
					}
				}
			}

			for (unsigned int k=0;k<packedRects.size();k++)
			{
				TmpGlyphInfo const &g = glyphs[packedRects[k].id];
				rbp::Rect const &out = packedRects[k];

				// insert into output.
				if (outBmp)
				{
					inki::font_glyph fg;
					fg.glyph = font->characters[packedRects[k].id];
					
					int outline = font->outline_width;
					
					fg.u0 = float(out.x + border - outline) / float(out_width);
					fg.v0 = float(out.y + border - outline) / float(out_height);
					fg.u1 = float(out.x + border + g.w + outline) / float(out_width);
					fg.v1 = float(out.y + border + g.h + outline) / float(out_height);
					fg.pixel_width = g.w + 2 * outline;
					fg.pixel_height = g.h + 2 * outline;
					fg.bearing_x = g.bearingX - outline * 64;
					fg.bearing_y = - g.bearingY - outline * 64;
					fg.advance = g.advance;
					up.glyphs.push_back(fg);

					for (int y=0;y<g.h;y++)
					{
						for (int x=0;x<g.w;x++)
						{
							outBmp[out_width * (out.y + y + border) + (out.x + x + border)] = g.data[g.w * y + x] * 0x01000000 | 0xffffff;
						}
					}
				}

				totalGlyphPixelData += g.w * g.h;
				totalGlyphs++;

				if (font->output_pixel_data)
				{
					if (g.w > 255 || g.h > 255)
					{
						RECORD_WARNING(info->record, "Ignoring glyph with dimensions " << g.w << "x" << g.h << " because u8 is used for size in output");
						continue;
					}

					// These do not have any u/v data, only the pixel data itself for run-time atlas generation/rendering.
					inki::font_glyph_pix_data pd;
					pd.glyph = font->characters[packedRects[k].id];
					pd.pixel_width = g.w;
					pd.pixel_height = g.h;
					pd.bearing_x = g.bearingX;
					pd.bearing_y = - g.bearingY;
					pd.advance = g.advance;
					for (int r=0;r<g.h;r++)
					{
						int ofs = r * g.w;
						row_cache_add(&cache, &g.data[ofs], g.w);
	
						for (int c=0;c<g.w;c++)
							pd.pixel_data.push_back(g.data[ofs + c]);
					}
					up.pix_glyphs.push_back(pd);
				}
			}

			for (int i=0;i!=glyphs.size();i++)
				delete [] glyphs[i].data;

			// create & insert texture.
			if (outBmp)
			{
				std::stringstream ss;
				ss << "i" << sz << "_px" << font->pixel_sizes[sz] << "_glyphs";
				
				if (font->outline_width > 0)
				{
					unsigned int *temp = new unsigned int[out_width * out_height];
					unsigned int *outline = new unsigned int[out_width * out_height];
					
					memcpy(outline, outBmp, out_width * out_height * 4);
					for (int i=0;i<font->outline_width;i++)
					{
						make_outline(temp, outline, out_width, out_height);
						memcpy(outline, temp, out_width * out_height * 4);
					}
					
					blend_outline(outBmp, outline, outBmp, out_width, out_height);
					delete [] temp;
					delete [] outline;
				}

				kosmos::pngutil::write_buffer wb = kosmos::pngutil::write_to_mem(outBmp, out_width, out_height);
				std::string output_atlas_path = putki::builder::store_resource_tag(info, "out.png", wb.output, wb.size);
				::free(wb.output);

				// create new texture.
				putki::ptr<inki::texture> texture = putki::builder::create_build_output<inki::texture>(info, ss.str().c_str());
				texture->source = output_atlas_path;
				texture->configuration = font->texture_configuration;

				// give font the texture.

				up.output_texture = texture;

				// add it so it will be built.
				delete [] outBmp;
			}

			font->outputs.push_back(up);
		}

		if (font->output_pixel_data)
		{
			// post processing
			font->rle_data.clear();
			row_cache_compress(&cache, &font->rle_data);

			RECORD_INFO(info->record, "Font contains " << totalGlyphs << " and total pixel data " << totalGlyphPixelData);
			RECORD_INFO(info->record, "Pixel data RLE compressed to " << font->rle_data.size());

			for (int i=0;i<font->outputs.size();i++)
			{
				inki::font_output *out = &font->outputs[i];
				for (int j=0;j!=out->pix_glyphs.size();j++)
				{
					inki::font_glyph_pix_data*pd = &out->pix_glyphs[j];

					for (int h = 0; h<pd->pixel_height; h++)
					{
						int ofs = h * pd->pixel_width;
						row *r = row_cache_add(&cache, &pd->pixel_data[ofs], pd->pixel_width);
						if (r && (r->comp_data_begin + r->comp_data_size) > font->rle_data.size())
						{
							RECORD_ERROR(info->record, "Internal build error in glyph cache, row_cache_add returned bad on known glyph row.")
						}

						if (!r)
						{
							pd->rle_data_begin.push_back(0);
							pd->rle_data_length.push_back(0);
						}
						else
						{
							pd->rle_data_begin.push_back(r->comp_data_begin);
							pd->rle_data_length.push_back(r->comp_data_size);
						}
					}

					pd->pixel_data.clear();
				}
			}

		}

cleanup:
		putki::builder::free_resource(&res);
		FT_Done_FreeType(ft);
		return true;
	}
}

void register_font_builder(putki::builder::data *builder)
{
	putki::builder::handler_info info[1] = {
		{ inki::font::type_id(), "font-builder-1", build_font, 0 }
	};
	putki::builder::add_handlers(builder, &info[0], &info[1]);
}