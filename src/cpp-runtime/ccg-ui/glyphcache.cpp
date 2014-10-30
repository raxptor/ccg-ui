#include "glyphcache.h"

#include <kosmos/render/alloc2d.h>
#include <kosmos/glwrap/gl.h>
#include <kosmos/log/log.h>

#include <unordered_map>

namespace ccgui
{
	namespace glyphcache
	{
		struct alloc
		{
			float u0, v0, u1, v1;
		};

		typedef std::unordered_map<void *, alloc> allocmap_t;

		struct cache_t
		{
			int width, height;
			kosmos::alloc2d::data *alloc;
			kosmos::render::texture_ref *tex;
			allocmap_t glyphs;
			GLuint texid;
		};

		enum
		{
			NUM_CACHES = 1
		};

		struct data
		{
			cache_t cache[NUM_CACHES];
		};

		void init_cache(cache_t *d, int width, int height)
		{
			d->width = width;
			d->height = height;
			d->alloc = kosmos::alloc2d::create(width, height);

			glGenTextures(1, &d->texid);
			glBindTexture(GL_TEXTURE_2D, d->texid);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

			d->tex = kosmos::render::make_ref(d->texid);
		}

		void free_cache(cache_t *d)
		{
			kosmos::render::unload_texture(d->tex);
			kosmos::alloc2d::free(d->alloc);
		}

		// it will allocate multiples of width x height
		data* create(int width, int height)
		{
			data *d = new data();
			for (int i=0;i<NUM_CACHES;i++)
				init_cache(&d->cache[i], width, height);
			return d;
		}

		void free(data *d)
		{
			for (int i=0;i<NUM_CACHES;i++)
				free_cache(&d->cache[i]);
			delete d;
		}

		// always returns something.
		bool get(data *d, void *handle, float *u, float *v, kosmos::render::texture_ref **ref)
		{
			for (int i=0;i<NUM_CACHES;i++)
			{
				cache_t *c = &d->cache[i];
				allocmap_t::iterator j = c->glyphs.find(handle);
				if (j != c->glyphs.end())
				{
					u[0] = j->second.u0;
					u[1] = j->second.u1;
					v[0] = j->second.v0;
					v[1] = j->second.v1;
					*ref = c->tex;
					return true;
				}
			}

			return false;
		}

		bool insert(data *d, void *handle, unsigned char *alpha, int width, int height)
		{
			// round up by 4.
			const int border = 1;

			int realh = height + 2*border;
			int w = width + 2*border;

			int pad = 3; // must be 2^n-1
			if (height > 24)
				pad = 7;
			if (height > 64)
				pad = 31;

			for (int waste=0;waste<8;waste++)
			{
				int h = (realh + pad + 4*waste) & (~pad); // waste a little to make allocation easier

				for (int i=0;i<NUM_CACHES;i++)
				{
					cache_t *c = &d->cache[i];

					int U[2], V[2];
					if (kosmos::alloc2d::alloc(c->alloc, w, h, U, V))
					{
						unsigned char pixbuf[4*65536];
						if (w * h > sizeof(pixbuf))
						{
							KOSMOS_ERROR("Glyph size is too large " << w << "x" << h)
							return false;
						}

						alloc tmp;
						tmp.u0 = float(U[0] + border) / c->width;
						tmp.u1 = float(U[0] + border + width) / c->width;
						tmp.v0 = float(V[0] + border) / c->height;
						tmp.v1 = float(V[0] + border + height) / c->height;

						unsigned char *wp = pixbuf;
						for (int y=0;y<h;y++)
						{
							for (int x=0;x<w;x++)
							{
								// write 0 for border, otherwise premultiplied alpha white
								if (x < border || x >= (w - border) || y < border || y >= (realh - border))
									wp[0] = wp[1] = wp[2] = wp[3] = 0;
								else
									wp[0] = wp[1] = wp[2] = wp[3] = *alpha++;
								
								wp += 4;
							}
						}

						glBindTexture(GL_TEXTURE_2D, d->cache->texid);
						glTexSubImage2D(GL_TEXTURE_2D, 0, U[0], V[0], w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixbuf);
						glBindTexture(GL_TEXTURE_2D, 0);

						c->glyphs.insert(allocmap_t::value_type(handle, tmp));
						return true;
					}
				}
			}

			//
			KOSMOS_WARNING("Glyph alloc failed!")
			return false;
		}

		void update(data*)
		{

		}
	}
}
