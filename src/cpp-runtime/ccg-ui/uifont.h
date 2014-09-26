#ifndef __CCGUI_FONT_H__
#define __CCGUI_FONT_H__

namespace outki
{
	struct FontGlyph;
	struct FontOutput;
	struct Font;
}

namespace kosmos
{
	namespace render2d
	{
		struct stream;
	}
}

namespace ccgui
{
	namespace uifont
	{
		struct layout_glyph
		{
			float x0, y0, x1, y1;
			const outki::FontGlyph *glyph;
		};

		struct layout_data
		{
			unsigned int glyphs_size;
			layout_glyph *glyphs;
			unsigned int lines;
			float minx, miny, maxx, maxy;
			float face_y0, face_y1;
			const outki::FontOutput *fontdata;
		};

		layout_data *layout_make(outki::Font *font, const char *text, float pixel_size, int max_width = -1, float rendering_scale_hint=1);
		void layout_draw(kosmos::render2d::stream *stream, layout_data *layout, float x, float y, unsigned long color);
		void layout_draw_align(kosmos::render2d::stream *stream, layout_data *layout, float x0, float y0, float x1, float y1, int vertical_align, int horizontal_align, unsigned long color);
		void layout_free(layout_data *layout);
	}
}

#endif
