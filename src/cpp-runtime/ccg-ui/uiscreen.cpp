#include "uiscreen.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/ccg-ui/Elements.h>
#include <kosmos/render/render.h>
#include <ccg-ui/uielement.h>
#include <ccg-ui/uiwidget.h>
#include <ccg-ui/elements/builtins.h>

#include <vector>
#include <math.h>
#include <string.h>

namespace ccgui
{
	namespace uiscreen
	{
		struct instance
		{
			outki::UIScreen *data;
			uiwidget::instance *root;
			element_handler_set *handlers;
			bool owns_handlers;
		};

		instance * create(outki::UIScreen *screen, element_handler_set *handlers)
		{
			instance *inst = new instance();
			inst->handlers = handlers;
			inst->owns_handlers = true;
			inst->data = screen;
			
			if (!inst->handlers)
			{
				inst->handlers = create_element_handler_set();
				inst->owns_handlers = true;
				add_builtin_element_handlers(inst->handlers);
			}
			
			renderinfo ri;
			ri.screen = inst;
			ri.context = 0;
			ri.handlers = inst->handlers;
			inst->root = uiwidget::create(screen->Root, &ri);
			return inst;
		}

		void free(instance *r)
		{
			uiwidget::free(r->root);
			if (r->owns_handlers)
				free_element_handler_set(r->handlers);
			delete r;
		}

		void draw(instance *d, uicontext *context, float x0, float y0, float x1, float y1)
		{
			LIVE_UPDATE(&d->data);
			LIVE_UPDATE(&d->data->Config);
			LIVE_UPDATE(&d->data->Root);

			float _x0 = x0;
			float _y0 = y0;
			float _x1 = x1;
			float _y1 = y1;

			if (d->data->Config->PreserveLayoutAspect)
			{
				const float xs = (x1 - x0) / d->data->Root->width;
				const float ys = (y1 - y0) / d->data->Root->height;
				const float s = xs < ys ? xs : ys;
				const float w = s * d->data->Root->width;
				const float h = s * d->data->Root->height;

				_x0 = floorf((x0 + x1 - w) / 2.0f);
				_y0 = floorf((y0 + y1 - h) / 2.0f);
				_x1 = _x0 + w;
				_y1 = _y0 + h;
			}

			renderinfo ri;
			ri.screen = d;
			ri.context = context;
			ri.handlers = d->handlers;
			
			uiwidget::layout(d->root, &ri, _x0, _y0, _x1, _y1);
			uiwidget::update(d->root, &ri);
			uiwidget::draw(d->root, &ri);
		}

		bool resolve_texture(instance *d, outki::Texture *texture, resolved_texture * out_resolved, float u0, float v0, float u1, float v1)
		{
			if (!texture)
			{
				out_resolved->texture = 0;
				return false;
			}
			
			for (unsigned int i=0;i<d->data->Atlases_size;i++)
			{
				outki::Atlas *atlas = d->data->Atlases[i];
				if (atlas)
				{
					for (unsigned int j=0;j<atlas->Outputs_size;j++)
					{
						const outki::AtlasOutput *output = &atlas->Outputs[j];
						if (output->Scale != 1.0f)
						{
							continue;
						}

						for (unsigned int k=0;k<output->Entries_size;k++)
						{
							const outki::AtlasEntry *entry = &output->Entries[k];
							if (!strcmp(entry->id, texture->id))
							{
								out_resolved->u0 = entry->u0 + (entry->u1 - entry->u0) * u0;
								out_resolved->v0 = entry->v0 + (entry->v1 - entry->v0) * v0;
								out_resolved->u1 = entry->u0 + (entry->u1 - entry->u0) * u1;
								out_resolved->v1 = entry->v0 + (entry->v1 - entry->v0) * v1;
								out_resolved->texture = kosmos::render::load_texture(output->Texture);
								return true;
							}
						}
					}
				}
			}

			out_resolved->u0 = u0;
			out_resolved->v0 = v0;
			out_resolved->u1 = u1;
			out_resolved->v1 = v1;
			
			if (texture->Output)
			{
				out_resolved->texture = kosmos::render::load_texture(texture);
				return true;
			}
			else
			{
				out_resolved->texture = 0;
				return false;
			}
		}

	}
}
