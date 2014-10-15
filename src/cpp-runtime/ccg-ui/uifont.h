#ifndef __CCGUI_FONT_H__
#define __CCGUI_FONT_H__

namespace outki
{
	struct FontOutput;
	struct Font;
}

namespace kosmos
{
	namespace render2d { struct stream; }
	namespace render { struct texture_ref; }
}

namespace ccgui
{
	namespace glyphcache { struct data; }
	namespace uifont
	{
		struct layout_glyph
		{
			float x0, y0, x1, y1;
			float u[2], v[2];
			kosmos::render::texture_ref *tex;
		};

		struct layout_data
		{
			unsigned int glyphs_size;
			bool pixel_snapped;
			layout_glyph *glyphs;
			unsigned int lines;
			float minx, miny, maxx, maxy;
			float face_y0, face_y1;
			const outki::FontOutput *fontdata;
		};

		layout_data *layout_make(outki::Font *font, glyphcache::data *cache, const char *text, float pixel_size, int max_width = -1, float rendering_scale_hint=1);
		void layout_draw(kosmos::render2d::stream *stream, layout_data *layout, float x, float y, unsigned long color);
		void layout_draw_align(kosmos::render2d::stream *stream, layout_data *layout, float x0, float y0, float x1, float y1, int vertical_align, int horizontal_align, unsigned long color);
		void layout_free(layout_data *layout);
	}
}

#endif
