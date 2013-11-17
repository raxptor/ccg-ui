#pragma once

#include <outki/types/ccg-ui/Screen.h>
#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/ccg-renderer.h>
#include <ccg-ui/uiscreen.h>

namespace ccgui
{
	namespace uiscreen
	{
		struct renderinfo;
	}
	
	namespace uiwidget
	{
		struct instance;
		
		struct element_layout
		{
			float x0, y0, x1, y1;
		};

		struct element_handler
		{
			virtual void destroy() { };
			virtual void on_layout(outki::UIElement *element, element_layout *layout) { };
			virtual void update(outki::UIElement *element, uiscreen::renderinfo *rinfo, element_layout *layout) { };
			virtual void draw(outki::UIElement *element, uiscreen::renderinfo *rinfo, element_layout *layout) { };
		};
	
		struct widget_handler
		{
			virtual element_handler* get_element_handler(instance *widgetInst, widget_handler *parent_handler, outki::UIElement *element) = 0;
		};
							
		instance *create(outki::UIWidget *screen, widget_handler *optional_handler);
		void layout(instance *d, float x0, float y0, float x1, float y1);
		void update(instance *d, uiscreen::renderinfo *context);
		void draw(instance *d, uiscreen::renderinfo *rinfo);
		void free(instance *r);
	}

}