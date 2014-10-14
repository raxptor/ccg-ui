#include "uiwidget.h"

#include <putki/liveupdate/liveupdate.h>

#include <ccg-ui/uielement.h>
#include <ccg-ui/elements/builtins.h>

#include <memory.h>
#include <stdio.h>

namespace ccgui
{
	namespace uiwidget
	{
		struct element_inst
		{
			// layout.
			outki::UIElement *element;
			element_handler_def *fns;
			element_layout layout;
			void *data;
		};

		struct instance
		{
			outki::UIWidget *widget;
			element_inst *elements;
			unsigned int elements_size;
		};

		instance *create(outki::UIWidget *widget, uiscreen::renderinfo *rinfo)
		{
			instance *d = new instance;
			d->widget = widget;

			unsigned int element_count = 0;
			for (unsigned int i=0;i<widget->layers_size;i++)
				element_count += widget->layers[i].elements_size;

			d->elements_size = element_count;
			d->elements = new element_inst[element_count];
			memset(d->elements, 0x00, sizeof(element_inst) * element_count);
			
			element_inst *cur = d->elements;
			for (unsigned int i=0;i<widget->layers_size;i++)
			{
				for (unsigned int j=0;j<widget->layers[i].elements_size;j++)
				{
					cur->element = widget->layers[i].elements[j];
					cur->fns = ccgui::get_element_handler(rinfo->handlers, cur->element->rtti_type_ref());
					cur->data = 0;
					if (cur->fns)
					{
						if (!(cur->data = cur->fns->init(rinfo, cur->element, 0)))
							cur->data = cur;
					}
						
					++cur;
				}
			}
			return d;
		}

		void free(instance *r)
		{
			for (unsigned int i=0;i<r->elements_size;i++)
			{
				if (r->elements[i].fns)
					r->elements[i].fns->done(r->elements[i].data);
			}

			delete [] r->elements;
			delete r;
		}

		void layout(instance *d, uiscreen::renderinfo *rinfo, const element_layout *layout)
		{
			float expx = (layout->nsx1 - layout->nsx0) - d->widget->width;
			float expy = (layout->nsy1 - layout->nsy0) - d->widget->height;

			if (expx < 0)
			{
				expx = 0;
			}
			if (expy < 0)
			{
				expy = 0;
			}

			// Compute layouts.
			for (unsigned int i=0;i<d->elements_size;i++)
			{
				if (LIVE_UPDATE(&d->elements[i].element))
				{
					element_handler_def *def = d->elements[i].fns;
					if (def)
					{
						// re-init.
						def->done(d->elements[i].data);
						d->elements[i].data = def->init(rinfo, d->elements[i].element, 0);
					}
				}
				
				outki::UIElement *element = d->elements[i].element;
				if (!element)
				{
					continue;
				}

				element_layout& li = d->elements[i].layout;

				// layout in non-scaled
				const float x0 = layout->nsx0;
				const float y0 = layout->nsy0;
				const float x1 = layout->nsx1;
				const float y1 = layout->nsy1;

				li.nsx0 = x0 + element->layout.x + expx * element->expansion.x;
				li.nsy0 = y0 + element->layout.y + expy * element->expansion.y;
				li.nsx1 = li.nsx0 + element->layout.width  + expx * element->expansion.width;
				li.nsy1 = li.nsy0 + element->layout.height + expy * element->expansion.height;

				// scale layout
				li.x0 = rinfo->layout_scale * li.nsx0 + rinfo->layout_offset_x;
				li.y0 = rinfo->layout_scale * li.nsy0 + rinfo->layout_offset_y;
				li.x1 = rinfo->layout_scale * li.nsx1 + rinfo->layout_offset_x;
				li.y1 = rinfo->layout_scale * li.nsy1 + rinfo->layout_offset_y;
			}
			
			for (unsigned int i=0;i<d->elements_size;i++)
			{
				if (d->elements[i].fns && d->elements[i].fns->layout)
					d->elements[i].fns->layout(rinfo, d->elements[i].element, &d->elements[i].layout, d->elements[i].data);
			}
		}

		void update(instance *d, uiscreen::renderinfo *rinfo)
		{
			for (unsigned int i=0;i<d->elements_size;i++)
			{
				element_handler_def *def = d->elements[i].fns;
				if (def && def->update)
				{
					def->update(rinfo, d->elements[i].element, &d->elements[i].layout, d->elements[i].data);
				}
			}
		}

		void draw(instance *d, uiscreen::renderinfo *rinfo)
		{
			for (unsigned int i=0;i<d->elements_size;i++)
			{
				if (d->elements[i].fns)
				{
					d->elements[i].fns->draw(rinfo, d->elements[i].element, &d->elements[i].layout, d->elements[i].data);
				}
			}
		}

	}
}
