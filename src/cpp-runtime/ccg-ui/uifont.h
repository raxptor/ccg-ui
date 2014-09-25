#ifndef __CCGUI_FONT_H__
#define __CCGUI_FONT_H__

namespace outki
{
	struct FontGlyph;
	struct FontOutput;
	struct Font;
}

namespace ccgui
{
	namespace uifont
	{
		struct data;

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


		data *create(outki::Font *font);
		void free(data *);

		layout_data *layout_make(data *font, const char *text, float pixel_size, int max_width = -1, float rendering_scale_hint=1);
		void layout_draw(layout_data *layout, float x, float y);
		void layout_free(layout_data *layout);
	}
}

#endif
