#include <ccg-ui/uiwidget.h>
#include <ccg-ui/uielement.h>
#include <kosmos/render/render.h>
#include <putki/liveupdate/liveupdate.h>

#include <stdio.h>

namespace ccgui
{
/*
	struct button_handler : uiwidget::element_handler
	{
		button_handler() : m_fill(0)
		{
		}

		outki::UIFill *m_fill;

		// default.
		virtual void destroy() {
			delete this;
		}
		void on_layout(outki::UIElement *element, uiwidget::element_layout *layout) override {
		}

		// do the button logic.
		void update(outki::UIElement *element, uiscreen::renderinfo *rinfo, uiwidget::element_layout *layout) override
		{
			outki::UIButtonElement *button = (outki::UIButtonElement *)element;
			LIVE_UPDATE(&button->Style);

			uielement::button_logic(rinfo, this, layout->x0, layout->y0, layout->x1, layout->y1);

			if (button->Style)
			{
				m_fill = button->Style->Normal;

				if (uielement::is_mousepressed(rinfo->context, this))
				{
					m_fill = button->Style->Pressed;
				}
				else if (uielement::is_mouseover(rinfo->context, this))
				{
					m_fill = button->Style->Highlight;
				}

				LIVE_UPDATE(&m_fill);
			}
		}

		void draw(outki::UIElement *element, uiscreen::renderinfo *rinfo, uiwidget::element_layout *layout) override
		{
			if (m_fill)
			{
				uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, m_fill);
			}
		}
	};

	struct default_element_handler : uiwidget::element_handler
	{
		bool m_on_stack;

		default_element_handler(bool onstack) : m_on_stack(onstack)
		{

		}

		virtual void destroy()
		{
			if (!m_on_stack)
			{
				delete this;
			}
		}

		void on_layout(outki::UIElement *element, uiwidget::element_layout *layout) override
		{

		}

		void update(outki::UIElement *element, uiscreen::renderinfo *rinfo, uiwidget::element_layout *layout) override
		{

		}

		void draw(outki::UIElement *element, uiscreen::renderinfo *rinfo, uiwidget::element_layout *layout) override
		{
			switch (element->rtti_type_ref())
			{
				case outki::UIButtonElement::TYPE_ID:
				{
					outki::UIButtonElement *button = (outki::UIButtonElement *)element;
					LIVE_UPDATE(&button->Style);

					uielement::button_logic(rinfo, this, layout->x0, layout->y0, layout->x1, layout->y1);

					if (button->Style)
					{
						outki::UIFill *fill = button->Style->Normal;

						if (uielement::is_mousepressed(rinfo->context, this))
						{
							fill = button->Style->Pressed;
						}
						else if (uielement::is_mouseover(rinfo->context, this))
						{
							fill = button->Style->Highlight;
						}

						LIVE_UPDATE(&fill);

						uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, fill);
					}
				}
				break;
				case outki::UIFillElement::TYPE_ID:
				{
					outki::UIFillElement *fill = (outki::UIFillElement *) element;
					LIVE_UPDATE(&fill->fill);

					//uielement::draw_fill(rinfo, layout->x0, layout->y0, layout->x1, layout->y1, fill->fill);
				}
				break;
				case outki::UIBitmapElement::TYPE_ID:
				{
					outki::UIBitmapElement *bmp = (outki::UIBitmapElement *) element;

					uiscreen::resolved_texture rt;
					if (uiscreen::resolve_texture(rinfo->screen, bmp->texture, &rt, 0, 0, 1, 1))
					{
						kosmos::render::tex_rect(rt.texture, layout->x0, layout->y0, layout->x1, layout->y1,
										 rt.u0, rt.v0, rt.u1, rt.v1, 0xffffffff);
					}
				}
				break;
				default:
					break;
			}
		}
	};

	struct widget_element_handler : uiwidget::element_handler
	{
		uiwidget::instance *inst;

		widget_element_handler(outki::UIWidget *widget, uiwidget::widget_handler *handler)
		{
			inst = uiwidget::create(widget, handler);
		}

		void destroy() override
		{
			uiwidget::free(inst);
			delete this;
		}

		void on_layout(outki::UIElement *element, uiwidget::element_layout *layout) override
		{
			uiwidget::layout(inst, layout->x0, layout->y0, layout->x1, layout->y1);
		}

		void update(outki::UIElement *element, uiscreen::renderinfo *rinfo, uiwidget::element_layout *layout) override
		{
			uiwidget::update(inst, rinfo);
		}

		void draw(outki::UIElement *element, uiscreen::renderinfo *context, uiwidget::element_layout *layout) override
		{
			uiwidget::draw(inst, context);
		}
	};

	struct default_widget_handler : uiwidget::widget_handler
	{
		uiwidget::element_handler* get_element_handler(uiwidget::instance *widgetInst, uiwidget::widget_handler *parent_handler, outki::UIElement *element) override
		{
			// used for elements that don't require unique handler instances.
			static default_element_handler handler(true);

			if (outki::UIWidgetElement *widget = element->exact_cast<outki::UIWidgetElement>())
			{
				return new widget_element_handler(widget->widget, this);
			}

			switch (element->rtti_type_ref())
			{
				case outki::UIButtonElement::TYPE_ID:
					return new button_handler();
				default:
					return &handler;
			}

		}
	};

	uiwidget::widget_handler* get_default_widget_handler()
	{
		static default_widget_handler s;
		return &s;
	}
	*/
}