#ifndef __CCGUI_GLYPH_CACHE_H__
#define __CCGUI_GLYPH_CACHE_H__

#include <outki/types/ccg-ui/Font.h>
#include <kosmos/render/render.h>

namespace ccgui
{
	namespace glyphcache
	{
		struct data;

		// it will allocate multiples of width x height
		data* create(int width, int height);
		void free(data*);

		bool get(data *, void *handle, float *u, float *v, kosmos::render::texture_ref **ref);
		bool insert(data *d, void *handle, unsigned char *alpha, int width, int height);

		void update(data*);
	}
}

#endif
