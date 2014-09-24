#ifndef __CCGUI_FONT_H__
#define __CCGUI_FONT_H__

#include <outki/types/ccg-ui/Font.h>

namespace ccgui
{
	namespace font
	{
		struct data;
		struct layout_data;

		data *create(outki::Font *font);
		void free(data *);

		layout_data *layout_make(data *font, const char *text, float pixel_size, int max_width = -1, float rendering_scale_hint=1);
		void layout_free(layout_data *layout);
	}
}

#endif
