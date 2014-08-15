#pragma once

#include <outki/types/ccg-ui/Elements.h>

namespace ccgui
{
	class render_api
	{
		public:

			render_api()
			{

			}

			virtual ~render_api()
			{

			}

			virtual void gradient_rect(float x0, float y0, float x1, float y1, unsigned int tl, unsigned int tr, unsigned int bl, unsigned int br) = 0;
			virtual void tex_rect(outki::Texture *texture, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, unsigned int color) = 0;

		private:
	};

	inline unsigned int col2int(outki::UIColor *c)
	{
		return (((unsigned int)c->a) << 24) | (((unsigned int)c->r) << 16)
		       | (((unsigned int)c->g) << 8) | (((unsigned int)c->b) << 0);
	}

}