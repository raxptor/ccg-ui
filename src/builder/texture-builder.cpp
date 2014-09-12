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

namespace {
	const char *builder_version = "texture-builder-1";
}

struct texbuilder : putki::builder::handler_i
{
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

		if (mode == inki::RESIZE_UNCROP_POW2SQUARE)
		{
			if (*out_width > *out_height)
			{
				*out_height = *out_width;
			}
			else if (*out_height > *out_width)
			{
				*out_width = *out_height;
			}
		}
	}

	virtual const char *version() {
		return builder_version;
	}

	virtual bool handle(putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj, putki::db::data *output, int obj_phase)
	{
		inki::Texture *texture = (inki::Texture *) obj;

		// this is used for atlas lookups later.
		texture->id = path;

		std::cout << "Processing texture [" << path << "] source <" << texture->Source << ">" << std::endl;

		// this object supplies its own defaults on initialisation
		inki::TextureConfiguration config;
		if (texture->Configuration)
		{
			putki::build_db::add_input_dependency(record, putki::db::pathof(input, texture->Configuration));
			config = *texture->Configuration;
		}

		putki::build_db::add_external_resource_dependency(record, texture->Source.c_str(), putki::resource::signature(builder, texture->Source.c_str()).c_str());

		// First load the information for the texture and fill in width & height
		ccgui::pngutil::loaded_png pnginfo;
		if (ccgui::pngutil::load_info(putki::resource::real_path(builder, texture->Source.c_str()).c_str(), &pnginfo))
		{
			texture->Width = texture->SourceWidth = pnginfo.width;
			texture->Height = texture->SourceHeight = pnginfo.height;
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

		int out_width, out_height;
		resize(outputFormat->ResizeMode, pnginfo.width, pnginfo.height, &out_width, &out_height);

		texture->Width = out_width;
		texture->Height = out_height;

		// note: assume uncrop
		const float u0 = 0;
		const float v0 = 0;
		const float u1 = float(pnginfo.width) / float(out_width);
		const float v1 = float(pnginfo.height) / float(out_height);

		if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatPathOnly::type_id())
		{
			// just write a "fake" png entry with the desired path.
			inki::TextureOutputFormatPathOnly *fmt = (inki::TextureOutputFormatPathOnly*) outputFormat;
			inki::TextureOutputPng *fakeTex = inki::TextureOutputPng::alloc();
			fakeTex->PngPath = std::string("Resources/") + path + fmt->FileEnding;
			fakeTex->parent.u0 = u0;
			fakeTex->parent.v0 = v0;
			fakeTex->parent.u1 = u1;
			fakeTex->parent.v1 = v1;
			texture->Output = &fakeTex->parent;

			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputPng::th(), fakeTex);
			putki::build_db::add_output(record, path_res.c_str(), builder_version);
		}

		// load
		ccgui::pngutil::loaded_png png;
		if (!ccgui::pngutil::load(putki::resource::real_path(builder, texture->Source.c_str()).c_str(), &png))
		{
			putki::builder::build_error(builder, "Failed to load source file!");
			return false;
		}
		
		if (outputFormat->PremultiplyAlpha)
		{
			for (int i=0;i<png.width*png.height;i++)
			{
				unsigned char *ptr = (unsigned char*)&png.pixels[i];
				ptr[1] = ptr[1] * ptr[0] / 255;
				ptr[2] = ptr[2] * ptr[0] / 255;
				ptr[3] = ptr[3] * ptr[0] / 255;
			}
		}

		// uncrop
		unsigned int *outData = png.pixels;
		if (out_width != png.width || out_height != png.height)
		{
			// only do uncrop
			std::cout << "[texture-builder] - uncropping to " << out_width << "x" << out_height << std::endl;
			outData = new unsigned int[out_width * out_height];

			for (int y=0;y<out_height;y++)
			{
				int srcy = y < png.height ? y : png.height - 1;
				unsigned int *srcLine = png.pixels + png.width * srcy;
				unsigned int *dstLine = outData + out_width * y;

				for (int x=0;x<png.width;x++)
					*dstLine++ = *srcLine++;

				// fill the rest with the last pixel
				for (int x=png.width;x<out_width;x++)
					*dstLine++ = *(srcLine-1);
			}
		}

		if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatOpenGL::type_id())
		{
			inki::TextureOutputOpenGL *glTex = inki::TextureOutputOpenGL::alloc();
			glTex->Width = out_width;
			glTex->Height = out_height;

			// RGBA
			for (int i=0;i<out_width * png.height;i++)
			{
				// RGBA
				glTex->Bytes.push_back((outData[i] >> 16) & 0xff);
				glTex->Bytes.push_back((outData[i] >>  8) & 0xff);
				glTex->Bytes.push_back((outData[i] >>  0) & 0xff);
				glTex->Bytes.push_back((outData[i] >> 24) & 0xff);
			}
			texture->Output = &glTex->parent;

			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputOpenGL::th(), glTex);
			putki::build_db::add_output(record, path_res.c_str(), builder_version);
		}
		else if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatPng::type_id())
		{
			// these are the direct load textures.
			inki::TextureOutputPng *pngObj = inki::TextureOutputPng::alloc();
			pngObj->PngPath = std::string("Resources/") + path + ".png";
			pngObj->parent.u0 = u0;
			pngObj->parent.v0 = v0;
			pngObj->parent.u1 = u1;
			pngObj->parent.v1 = v1;

			std::cout << "[TextureOutputFormatPng] - Source image [" << png.width << "x" << png.height << "] => [" << texture->Width << "x" << texture->Height << "]" << std::endl;

			ccgui::pngutil::write_to_output(builder, pngObj->PngPath.c_str(), outData, out_width, out_height);

			texture->Output = &pngObj->parent;

			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputPng::th(), pngObj);

			putki::build_db::add_output(record, path_res.c_str(), builder_version);
		}
		else if (outputFormat->rtti_type_ref() == inki::TextureOutputFormatJpeg::type_id())
		{
			// these are the direct load textures.
			inki::TextureOutputJpeg *jpgObj = inki::TextureOutputJpeg::alloc();
			jpgObj->JpegPath = std::string("Resources/") + path + ".jpeg";
			jpgObj->parent.u0 = u0;
			jpgObj->parent.v0 = v0;
			jpgObj->parent.u1 = u1;
			jpgObj->parent.v1 = v1;

			texture->Output = &jpgObj->parent;

			int buf_size = 4*1024*1024;
			char *databuffer = new char[buf_size];

			// swap order in-place in the png buffer (naughty)
			unsigned char *pngpixels = (unsigned char*)outData;
			for (unsigned int i=0;i<out_width*out_height;i++)
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

			if (!jpge::compress_image_to_jpeg_file_in_memory(databuffer, buf_size, out_width, out_height, 4, (unsigned char*)outData, p))
			{
				putki::builder::build_error(builder, "JPEG compression failed");
			}
			else
			{
				std::cout << "[jpeg] compressed " << out_width << "x" << out_height << " to " << buf_size << " bytes." << std::endl;
				putki::resource::save_output(builder, jpgObj->JpegPath.c_str(), databuffer, buf_size);
			}

			delete [] databuffer;

			std::string path_res(path);
			path_res.append("_out");
			putki::db::insert(output, path_res.c_str(), inki::TextureOutputJpeg::th(), jpgObj);
			putki::build_db::add_output(record, path_res.c_str(), builder_version);
		}

		if (outData != png.pixels)
		{
			delete [] outData;
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
