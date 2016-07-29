#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/log.h>
#include <putki/builder/db.h>

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

struct TmpGlyphInfo
{
	int w, h;
	int bearingX, bearingY;
	int advance;
	unsigned char *data;
};

namespace {
    
    // might cut a bit or two of precision here and there.
    int compress_glyph(std::vector<unsigned char> const & input, unsigned char *output)
    {
        const int expand = 64;
        bool start_zeroes = true;
        
        std::vector<bool> tmp;
        tmp.reserve(1024);
        for (int j=0;j<input.size();j++)
        {
            const int ones = input[j] * expand / 255;
            const int zeroes = expand - ones;
            
            if (start_zeroes)
            {
                for (int k=0;k<zeroes;k++) tmp.push_back(false);
                for (int k=0;k<ones;k++) tmp.push_back(true);
                if (ones > 0)
                    start_zeroes = false;
            }
            else
            {
                for (int k=0;k<ones;k++) tmp.push_back(true);
                for (int k=0;k<zeroes;k++) tmp.push_back(false);
                if (zeroes > 0)
                    start_zeroes = true;
            }
        }
        
        int outsize = 0;
        int runlength = 0;
        int old_val = 0;
        std::vector<int> lengths;
        lengths.reserve(1024);
        for (int i=0;i<tmp.size();i++)
        {
            if (old_val == tmp[i])
            {
                runlength++;
            }
            else
            {
                lengths.push_back(runlength);
                runlength = 1;
                old_val = tmp[i];
            }
        }
        
        for (int s=0;s<lengths.size();s++)
        {
            int val = lengths[s];
            while (val > 0x7F)
            {
                output[outsize++] = 0x80 | (val & 0x7F);
                val = val >> 7;
            }
            output[outsize++] = val & 0x7F;
        }
        
        return outsize;
    }
    
    
	const char *builder_version = "font-builder-3b";
	
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
}

struct fontbuilder : putki::builder::handler_i
{
	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::Font *font = (inki::Font *) obj;

		RECORD_INFO(record, "Source is " << font->Source)
		
		if (font->Latin1)
		{
			for (int i='a';i<='z';i++)
				font->Characters.push_back(i);
			for (int i='A';i<='Z';i++)
				font->Characters.push_back(i);
			for (int i='0';i<='9';i++)
				font->Characters.push_back(i);

			const char *special = "!()#?:/\\<>[] .,";
			for (int i=0;i<strlen(special);i++)
				font->Characters.push_back(special[i]);
		}
		
		if (font->TradChinese)
		{
			for (int i=0x4E00;i<0x6000;i++)
				font->Characters.push_back(i);
		}

		putki::build_db::add_external_resource_dependency(record, font->Source.c_str(), putki::resource::signature(builder, font->Source.c_str()).c_str());

		const char *fnt_data;
		long long fnt_len;
		if (!putki::resource::load(builder, font->Source.c_str(), &fnt_data, &fnt_len))
		{
			RECORD_WARNING(record, "Could not load font at [" << font->Source << "]!")
			return false;
		}

		FT_Library ft;
		if (FT_Init_FreeType(&ft))
		{
			RECORD_WARNING(record, "Could not initialize freetype");
			delete [] fnt_data;
			return false;
		}

		FT_Face face;
		if (FT_New_Memory_Face(ft, (const FT_Byte *)fnt_data, (FT_Long)fnt_len, 0, &face))
		{
			RECORD_WARNING(record, "Could not load font face");
			delete [] fnt_data;
			FT_Done_FreeType(ft);
			return false;
		}

		int totalGlyphs = 0;
		int totalGlyphPixelData = 0;

		for (unsigned int sz=0;sz<font->PixelSizes.size();sz++)
		{
			if (FT_Set_Pixel_Sizes(face, 0, font->PixelSizes[sz]))
			{
				RECORD_WARNING(record, "Could not set char or pixel size.");
				goto cleanup;
			}

			std::vector< rbp::InputRect > packs;
			std::vector<TmpGlyphInfo> glyphs;

			inki::FontOutput up;
			up.PixelSize = font->PixelSizes[sz];

			up.BBoxMinY = 1000000;
			up.BBoxMaxY = -100000;

			int border = 2 + font->OutlineWidth;

			for (unsigned int i=0;i<font->Characters.size();i++)
			{
				int idx = FT_Get_Char_Index(face, font->Characters[i]);
				if (FT_Load_Glyph(face, idx, FT_LOAD_NO_BITMAP))
				{
					//RECORD_WARNING(record, "Could not load glyph face.");
					continue;
				}

				if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
				{
					//RECORD_WARNING(record, "Could not render glyph.");
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
				if (y0 < up.BBoxMinY)
				{
					up.BBoxMinY = y0;
				}
				if (y1 > up.BBoxMaxY)
				{
					up.BBoxMaxY = y1;
				}

				for (unsigned int j=0;j<font->Characters.size();j++)
				{
					FT_Vector kerning;
					if (!FT_Get_Kerning(face, font->Characters[i], font->Characters[j], FT_KERNING_DEFAULT, &kerning))
					{
						if (kerning.x != 0)
						{
							up.KerningCharL.push_back(font->Characters[i]);
							up.KerningCharR.push_back(font->Characters[j]);
							up.KerningOfs.push_back(kerning.x);
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

			if (font->TextureConfiguration)
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
					inki::FontGlyph fg;
					fg.glyph = font->Characters[packedRects[k].id];
					
					int outline = font->OutlineWidth;
					
					fg.u0 = float(out.x + border - outline) / float(out_width);
					fg.v0 = float(out.y + border - outline) / float(out_height);
					fg.u1 = float(out.x + border + g.w + outline) / float(out_width);
					fg.v1 = float(out.y + border + g.h + outline) / float(out_height);
					fg.pixelWidth = g.w + 2 * outline;
					fg.pixelHeight = g.h + 2 * outline;
					fg.bearingX = g.bearingX - outline * 64;
					fg.bearingY = - g.bearingY - outline * 64;
					fg.advance = g.advance;
					up.Glyphs.push_back(fg);

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

				if (font->OutputPixelData)
				{
					if (g.w > 255 || g.h > 255)
					{
						RECORD_WARNING(record, "Ignoring glyph with dimensions " << g.w << "x" << g.h << " because u8 is used for size in output");
						continue;
					}

					// These do not have any u/v data, only the pixel data itself for run-time atlas generation/rendering.
					inki::FontGlyphPixData pd;
					pd.glyph = font->Characters[packedRects[k].id];
					pd.pixelWidth = g.w;
					pd.pixelHeight = g.h;
					pd.bearingX = g.bearingX;
					pd.bearingY = - g.bearingY;
					pd.advance = g.advance;
                    
					for (int r=0;r<g.h;r++)
					{
                        pd.pixelData.reserve(g.w * g.h);
						int ofs = r * g.w;
						for (int c=0;c<g.w;c++)
							pd.pixelData.push_back(g.data[ofs + c]);
					}
					up.PixGlyphs.push_back(pd);
				}
			}

			for (int i=0;i!=glyphs.size();i++)
				delete [] glyphs[i].data;

			// create & insert texture.
			if (outBmp)
			{
				std::stringstream ss;
				ss << path << "_i" << sz << "_px" << font->PixelSizes[sz] << "_glyphs";
				
				if (font->OutlineWidth > 0)
				{
					unsigned int *temp = new unsigned int[out_width * out_height];
					unsigned int *outline = new unsigned int[out_width * out_height];
					
					memcpy(outline, outBmp, out_width * out_height * 4);
					for (int i=0;i<font->OutlineWidth;i++)
					{
						make_outline(temp, outline, out_width, out_height);
						memcpy(outline, temp, out_width * out_height * 4);
					}
					
					blend_outline(outBmp, outline, outBmp, out_width, out_height);
					delete [] temp;
					delete [] outline;
				}

				std::string outpath = ss.str();
				std::string output_atlas_path = ss.str() + ".png";
				output_atlas_path = ccgui::pngutil::write_to_temp(builder, output_atlas_path.c_str(), outBmp, out_width, out_height);
				putki::builder::touched_temp_resource(builder, output_atlas_path.c_str());

				// create new texture.
				inki::Texture *texture = inki::Texture::alloc();
				texture->Source = output_atlas_path;
				texture->Configuration = font->TextureConfiguration;

				// give font the texture.

				up.OutputTexture = texture;

				// add it so it will be built.
				add_output(context, record, outpath.c_str(), texture);
				delete [] outBmp;
			}

			font->Outputs.push_back(up);
		}

		if (font->OutputPixelData)
		{
            unsigned int total = 0;
			for (int i=0;i<font->Outputs.size();i++)
			{
				inki::FontOutput *out = &font->Outputs[i];
				for (int j=0;j!=out->PixGlyphs.size();j++)
				{
					inki::FontGlyphPixData *pd = &out->PixGlyphs[j];
                    
                    printf("compressing %d/%d\n", i, j);
                    
                    unsigned char tmp[256*1024];
                    int bt = compress_glyph(   V.snaoe*STaoeräårlecöä+"5+67ygfg'pd->pixelData, tmp);
                    if (bt > sizeof(tmp))
                    {
                        RECORD_ERROR(record, "Smashed stack!");
                        break;
                    }
                    
//                    for (int k=0;k<bt;k++)
//                      pd->compressedPixelData.push_back(tmp[k]);
                    for (int k=0;k<pd->pixelData.size();k++)
                        pd->compressedPixelData.push_back(pd->pixelData[k]);

//                    pd->pixelData.clear();
                    total += bt;
				}
			}
            
            RECORD_INFO(record, "Font contains " << totalGlyphs << " glyphs and total pixel data " << totalGlyphPixelData);
            RECORD_INFO(record, "Pixel data compresses to " << total << " bytes");
		}

cleanup:
		delete [] fnt_data;
		FT_Done_FreeType(ft);
		return false;
	}
};

void register_font_builder(putki::builder::data *builder)
{
	static fontbuilder fb;
	putki::builder::add_data_builder(builder, "Font", &fb);
}
