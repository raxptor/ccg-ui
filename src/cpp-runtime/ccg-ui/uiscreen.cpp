#include "uiscreen.h"

#include <putki/liveupdate/liveupdate.h>
#include <outki/types/ccg-ui/Elements.h>
#include <kosmos/render/render.h>
#include <kosmos/render/render2d.h>
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

		void draw(instance *d, kosmos::render2d::stream *stream, uicontext *context, float x0, float y0, float x1, float y1)
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
			ri.render_scaling_hint = 1.0f;
			ri.stream = stream;

			bool pushed_matrix = false;

			if (d->data->Config->ScaleMode == outki::ScaleMode_Prop_Transform || d->data->Config->ScaleMode == outki::ScaleMode_Prop_Layout)
			{
				float width = (_x1 - _x0) / d->data->Root->width;
				float height = (_y1 - _y0) / d->data->Root->height;
				float scale = width < height ? width : height;

				if (d->data->Config->SnapScale)
				{
					// snap scale to nice values
					const int num =7;
					const float snaps[7] = {8.0f, 4.0f, 2.0f, 1.0f, 0.5f, 0.25f, 0.125f};
					for (int i=0;i<num;i++)
					{
						if (scale > snaps[i])
						{
							scale = snaps[i];
							break;
						}
					}
				}

				if (d->data->Config->ScaleMode == outki::ScaleMode_Prop_Transform)
				{
					pushed_matrix = true;
					kosmos::render2d::set_2d_transform(stream, scale, scale, (int)_x0/scale, (int)_y0/scale);

					ri.render_scaling_hint = scale;

					// compute un-scaled rect at 0,0 since above transform takes it to x0 y0
					_x1 = (_x1 - _x0) / scale;
					_y1 = (_y1 - _y0) / scale;
					_x0 = 0;
					_y0 = 0;
					scale = 1.0f;
				}

				ri.layout_scale = scale;
				ri.layout_offset_x = (int)(-(float)(_x0 * (scale - 1)));
				ri.layout_offset_y = (int)(-(float)(_y0 * (scale - 1)));
			}
			else
			{
				ri.layout_scale = 1.0f;
				ri.layout_offset_x = 0;
				ri.layout_offset_y = 0;
			}

			uiwidget::element_layout root_layout;
			root_layout.x0 = _x0;
			root_layout.y0 = _y0;
			root_layout.x1 = _x1;
			root_layout.y1 = _y1;

			root_layout.nsx0 = _x0;
			root_layout.nsy0 = _y0;
			root_layout.nsx1 = _x0 + (_x1 - _x0) / ri.layout_scale;
			root_layout.nsy1 = _y0 + (_y1 - _y0) / ri.layout_scale;

			uiwidget::layout(d->root, &ri, &root_layout);
			uiwidget::update(d->root, &ri);
			uiwidget::draw(d->root, &ri);

			if (pushed_matrix)
			{
				kosmos::render::pop_matrix();
			}
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
