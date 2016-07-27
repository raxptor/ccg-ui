#pragma once

#include <outki/types/ccg-ui/Elements.h>
#include <ccg-ui/uicontext.h>

namespace ccgui
{
	namespace uiscreen { struct instance; struct renderinfo; }
	namespace uiwidget { struct element_layout; }

	typedef void* (*element_init_fn)(uiscreen::renderinfo *rinfo, outki::ui_element *element, void *instance_data_inplace);
	typedef void  (*element_layout_fn)(uiscreen::renderinfo *rinfo, outki::ui_element *element, uiwidget::element_layout *layout, void *instance_data);
	typedef void  (*element_update_draw_fn)(uiscreen::renderinfo *rinfo, outki::ui_element *element, uiwidget::element_layout *layout, void *instance_data);
	typedef void  (*element_done_fn)(void *instance_data);
	
	struct element_handler_def
	{
		element_init_fn init;
		element_layout_fn layout;
		element_update_draw_fn update;
		element_update_draw_fn draw;
		element_done_fn done;
	};
	
	struct element_no_instance
	{
	};
	
	template<typename element_type, typename instance_type=element_no_instance>
	struct wrap_it
	{
		typedef instance_type* (*init_fn)(uiscreen::renderinfo *rinfo, element_type *element, instance_type *inplace);
		typedef void (*layout_fn)(uiscreen::renderinfo *rinfo, element_type *element, uiwidget::element_layout *layout, instance_type *instance_data);
		typedef void (*update_draw_fn)(uiscreen::renderinfo *rinfo, element_type *element, uiwidget::element_layout *layout, instance_type *instance_data);
		typedef void (*done_fn)(instance_type *t);
		
		static void* def_init(uiscreen::renderinfo *rinfo, outki::ui_element *element, void *inplace)
		{
			if (inplace)
				return 0;
			if (!sizeof(instance_type))
				return 0;
			else
				return new instance_type();
		}
		
		static void def_done(void *done)
		{
			delete (instance_type *)done;
		}
		
		void fill_handler(element_handler_def *def, init_fn init, layout_fn layout, update_draw_fn update,
		         update_draw_fn draw, done_fn done)
		{
			def->init = init ? (element_init_fn) init : def_init;
			def->layout = (element_layout_fn) layout;
			def->update = (element_update_draw_fn) update;
			def->draw = (element_update_draw_fn) draw;
			def->done = done ? (element_done_fn) done : def_done;
		}
	};
			
	struct element_handler_set;
	
	element_handler_set *create_element_handler_set();
	element_handler_def *get_element_handler(element_handler_set *set, int type);
	
	void set_element_handler(element_handler_set *set, int type, element_handler_def const & def);
	void free_element_handler_set(element_handler_set *set);
	
	template<typename A, typename B>
	void set_element_handler(
		element_handler_set *set,
		typename wrap_it<A, B>::init_fn init,
		typename wrap_it<A, B>::layout_fn layout,
		typename wrap_it<A, B>::update_draw_fn update,
		typename wrap_it<A, B>::update_draw_fn draw,
		typename wrap_it<A, B>::done_fn done)
	{
		element_handler_def def;
		wrap_it<A, B> wr0;
		wr0.fill_handler(&def, init, layout, update, draw, done);
		set_element_handler(set, A::type_id(), def);
	}
	
	namespace uielement
	{
		// generic components
		bool hittest(uiscreen::renderinfo *rinfo, float x, float y, float x0, float y0, float x1, float y1);
		bool is_mouseover(uicontext *context, element_id elId);
		bool is_mousepressed(uicontext *context, element_id elId);
		bool button_logic(uiscreen::renderinfo *rinfo, element_id elId, float x0, float y0, float x1, float y1);
		void draw_fill(uiscreen::renderinfo *rinfo, float x0, float y0, float x1, float y1, outki::ui_fill *fill);
	}
	
	inline unsigned int col2int(outki::ui_color const & col)
	{
		return (col.a << 24) | (col.b << 16) | (col.g << 8) | col.r;
	}
}
