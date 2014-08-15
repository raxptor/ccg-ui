#include "uiwidget.h"

#include <putki/liveupdate/liveupdate.h>
#include <ccg-ui/uielement.h>
#include <ccg-ui/defaulthandlers.h>

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
			element_handler *handler;
			element_layout layout;
		};

		struct instance
		{
			outki::UIWidget *widget;
			element_inst *elements;
			unsigned int elements_size;
		};

		instance *create(outki::UIWidget *widget, uiwidget::widget_handler *handler)
		{
			instance *d = new instance;
			d->widget = widget;

			unsigned int element_count = 0;
			for (unsigned int i=0;i<widget->layers_size;i++)
				element_count += widget->layers[i].elements_size;

			d->elements_size = element_count;
			d->elements = new element_inst[element_count];
			memset(d->elements, 0x00, sizeof(element_inst) * element_count);

			widget_handler *def_handler = get_default_widget_handler();
			widget_handler *use_handler = handler ? handler : def_handler;

			element_inst *cur = d->elements;
			for (unsigned int i=0;i<widget->layers_size;i++)
			{
				for (unsigned int j=0;j<widget->layers[i].elements_size;j++)
				{
					cur->element = widget->layers[i].elements[j];
					cur->handler = use_handler->get_element_handler(d, 0, cur->element);
					++cur;
				}
			}
			return d;
		}

		void free(instance *r)
		{
			for (unsigned int i=0;i<r->elements_size;i++)
			{
				if (r->elements[i].handler)
				{
					r->elements[i].handler->destroy();
				}
			}

			delete [] r->elements;
			delete r;
		}

		void layout(instance *d, float x0, float y0, float x1, float y1)
		{
			float expx = (x1 - x0) - d->widget->width;
			float expy = (y1 - y0) - d->widget->height;
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
				LIVE_UPDATE(&d->elements[i].element);
				outki::UIElement *element = d->elements[i].element;
				if (!element)
				{
					continue;
				}

				element_layout& li = d->elements[i].layout;
				li.x0 = x0 + element->layout.x + expx * element->expansion.x;
				li.y0 = y0 + element->layout.y + expy * element->expansion.y;
				li.x1 = li.x0 + element->layout.width  + expx * element->expansion.width;
				li.y1 = li.y0 + element->layout.height + expy * element->expansion.height;
			}

			for (unsigned int i=0;i<d->elements_size;i++)
			{
				if (d->elements[i].handler)
				{
					d->elements[i].handler->on_layout(d->elements[i].element, &d->elements[i].layout);
				}
			}
		}

		void update(instance *d, uiscreen::renderinfo *rinfo)
		{
			for (unsigned int i=0;i<d->elements_size;i++)
			{
				if (d->elements[i].handler)
				{
					d->elements[i].handler->update(d->elements[i].element, rinfo, &d->elements[i].layout);
				}
			}
		}

		void draw(instance *d, uiscreen::renderinfo *rinfo)
		{
			for (unsigned int i=0;i<d->elements_size;i++)
			{
				if (d->elements[i].handler)
				{
					d->elements[i].handler->draw(d->elements[i].element, rinfo, &d->elements[i].layout);
				}
			}
		}

	}
}
