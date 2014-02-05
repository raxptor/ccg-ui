#include <putki/builder/app.h>
#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <builder/pngutil.h>
#include <builder/jpge.h>

#include <iostream>

#include <inki/types/ccg-ui/Texture.h>

#include "textureconfig.h"

struct texbuilder : putki::builder::handler_i
{
	/* no support yet
	static void resize(inki::TextureResizeMode mode, int width, int height, int *out_width, int *out_height)
	{
		if (mode == inki::RESIZE_NONE)
		{
			*out_width = width;
			*out_height = height;
			return;
		}
		
		*out_width = 1;
		*out_height = 1;
		
		while (*out_width < width) *out_width *= 2;
		while (*out_height < height) *out_height *=2;
	
		if (mode == inki::RESIZE_POW2SQUARE)
		{
			if (*out_width > *out_height)
				*out_height = *out_width;
			else if (*out_height > *out_width)
				*out_width = *out_height;
		}	
	}
	*/

	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Texture *texture = (inki::Texture *) obj;
		
		// this is used for atlas lookups later.
		texture->id = path;

		std::cout << "Processing texture [" << path << "] source <" << texture->Source << ">" << std::endl;

		// this object supplies its own defaults on initialisation		
		inki::TextureConfiguration config;
		if (texture->Configuration)
			config = *texture->Configuration;
		
		// First load the information for the texture and fill in width & height
		ccgui::pngutil::loaded_png pnginfo;
		if (ccgui::pngutil::load_info(putki::resource::real_path(builder, texture->Source.c_str()).c_str(), &pnginfo))
		{
			texture->Width = pnginfo.width;
			texture->Height = pnginfo.height;
			ccgui::pngutil::free(&pnginfo);	
		}
		else
		{
			putki::builder::build_error(builder, "Could not load source file!");
			return false;
		}
		
		// If no output is needed
		if (!config.OutputFormat)
		{
			std::cout << " => Texture has no output format set. Skipping generation" << std::endl;
			return false;
		}
		
		// TODO: Check overrides for other platforms
		inki::TextureOutputFormat *outputFormat = config.OutputFormat;

		if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatPathOnly::type_id())
		{
			// just write a "fake" png entry with the desired path.
			inki::TextureOutputFormatPathOnly *fmt = (inki::TextureOutputFormatPathOnly*) outputFormat;		
			inki::TextureOutputPng *fakeTex = inki::TextureOutputPng::alloc();
			fakeTex->PngPath = std::string("Resources/") + path + fmt->FileEnding;
			
			texture->Width = pnginfo.width;
			texture->Height = pnginfo.height;
			texture->Output = &fakeTex->parent;
			
			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputPng::th(), fakeTex);
			putki::build_db::add_output(record, path_res.c_str());
		}
		
		// load 
		ccgui::pngutil::loaded_png png;
		if (!ccgui::pngutil::load(putki::resource::real_path(builder, texture->Source.c_str()).c_str(), &png))
		{
			putki::builder::build_error(builder, "Failed to load source file!");
			return false;
		}
		
		
		if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatOpenGL::type_id())
		{
			inki::TextureOutputOpenGL *glTex = inki::TextureOutputOpenGL::alloc();
			glTex->Width = png.width;
			glTex->Height = png.height;
			
			// RGBA
			for (int i=0;i<png.width * png.height;i++)
			{
				// RGBA
				glTex->Bytes.push_back((png.pixels[i] >> 16) & 0xff);
				glTex->Bytes.push_back((png.pixels[i] >>  8) & 0xff);
				glTex->Bytes.push_back((png.pixels[i] >>  0) & 0xff);
				glTex->Bytes.push_back((png.pixels[i] >> 24) & 0xff);
			}
			texture->Output = &glTex->parent;
			
			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputOpenGL::th(), glTex);
			putki::build_db::add_output(record, path_res.c_str());
		}
		else if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatPng::type_id())
		{
			// these are the direct load textures.
			inki::TextureOutputPng *pngObj = inki::TextureOutputPng::alloc();
			pngObj->PngPath = std::string("Resources/") + path + ".png";
			pngObj->parent.u0 = 0;
			pngObj->parent.v0 = 0;
			pngObj->parent.u1 = float(png.width) / float(texture->Width);
			pngObj->parent.v1 = float(png.height) / float(texture->Height);
			
			unsigned int *pixels_out = new unsigned int[texture->Width * texture->Height];
			memset(pixels_out, 0x00, texture->Width * texture->Height * sizeof(unsigned int));
			
			for (int y=0;y<png.height;y++)
			{
				unsigned int *dst_start = &pixels_out[y * texture->Width];
				unsigned int *src_start = ((unsigned int*)png.pixels) + (y * png.width);
				memcpy(dst_start, src_start, png.width * sizeof(unsigned int));
			}
			
			std::cout << "[TextureOutputFormatPng] - Source image [" << png.width << "x" << png.height << "] => [" << texture->Width << "x" << texture->Height << "]" << std::endl;
		
			ccgui::pngutil::write_to_output(builder, pngObj->PngPath.c_str(), pixels_out, texture->Width, texture->Height);
			delete [] pixels_out;

			texture->Output = &pngObj->parent;
			
			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputPng::th(), pngObj);

			putki::build_db::add_output(record, path_res.c_str());
		}
		else if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatJpeg::type_id())
		{
			// these are the direct load textures.
			inki::TextureOutputJpeg *jpgObj = inki::TextureOutputJpeg::alloc();
			jpgObj->JpegPath = std::string("Resources/") + path + ".jpeg";
			jpgObj->parent.u0 = 0;
			jpgObj->parent.v0 = 0;
			jpgObj->parent.u1 = 1; 
			jpgObj->parent.v1 = 1;
			
			texture->Output = &jpgObj->parent;

			int buf_size = 4*1024*1024;
			char *databuffer = new char[buf_size];
			
			// swap order in-place in the png buffer (naughty)
			unsigned char *pngpixels = (unsigned char*)png.pixels;
			for (unsigned int i=0;i<png.width*png.height;i++)
			{
				unsigned char t0 = pngpixels[0];
				unsigned char t1 = pngpixels[1];
				unsigned char t2 = pngpixels[2];
				unsigned char t3 = pngpixels[3];
				pngpixels[0] = t2;
				pngpixels[1] = t1;
				pngpixels[2] = t0;
				pngpixels[3] = t3;
				
				pngpixels += 4;
			}
			
			inki::TextureOutputFormatJpeg *fmt = (inki::TextureOutputFormatJpeg *) outputFormat;
			jpge::params p;
			p.m_quality = fmt->Quality;
			p.m_two_pass_flag = fmt->Twopass; 
			
			if (!jpge::compress_image_to_jpeg_file_in_memory(databuffer, buf_size, png.width, png.height, 4, (unsigned char*)png.pixels, p))
			{
				putki::builder::build_error(builder, "JPEG compression failed");
			}
			else
			{
				std::cout << "[jpeg] compressed " << png.width << "x" << png.height << " to " << buf_size << " bytes." << std::endl;
				putki::resource::save_output(builder, jpgObj->JpegPath.c_str(), databuffer, buf_size);
			}
			
			delete [] databuffer;
			
			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputJpeg::th(), jpgObj);
			putki::build_db::add_output(record, path_res.c_str());
		}

		ccgui::pngutil::free(&png);
		return false;
	}
};

void register_texture_builder(putki::builder::data *builder)
{
	static texbuilder fb;
	putki::builder::add_data_builder(builder, "Texture", putki::builder::PHASE_INDIVIDUAL, &fb);
}
