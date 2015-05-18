namespace CCGUI
{
	class UISliderElementRenderer : UIElementRenderer
	{
		UIFill m_fill;
		outki.UISliderElement m_element;
		UIRenderer.Texture m_knobTexture;
		UITouchInteraction m_ti = new CCGUI.UITouchInteraction();
		float m_knobWidth, m_knobHeight;
		float m_value, m_preDragValue;
		float m_dragStartX, m_dragStartY;
		bool m_dragTouch, m_dragMouse;
		
		public UISliderElementRenderer(outki.UISliderElement element)
		{
			m_fill = new UIFill(element.BackgroundFill);
			m_element = element;
			m_value = 0.5f;
		}
		
		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			m_fill.ResolveTextures(rctx);
			m_fill.OnLayout(rctx, elementLayout.x0, elementLayout.y0, elementLayout.x1, elementLayout.y1);
			if (m_element.Knob != null)
			{
				m_knobTexture = rctx.TextureManager.ResolveTexture(m_element.Knob, rctx.LayoutScale, 0, 0, 1, 1);
				m_knobWidth = m_element.Knob.SourceWidth;
				m_knobHeight = m_element.Knob.SourceHeight;
			}
		}
		
		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			m_fill.Draw(rctx, layout.x0, layout.y0, layout.x1, layout.y1);
			if (m_knobTexture == null)
				return;
				
			// Current knob
			float h_width = 0.5f * rctx.LayoutScale * m_knobWidth;
			float h_height = 0.5f * rctx.LayoutScale * m_knobHeight;
			float x = layout.x0 + m_value * (layout.x1 - layout.x0);
			float y = layout.y0 + m_value * (layout.y1 - layout.y0);
			float x0 = x - h_width;
			float y0 = y - h_height;
			float x1 = x + h_width;
			float y1 = y + h_height;
			UIRenderer.DrawTexture(m_knobTexture, x0, y0, x1, y1);

			float displayVal = m_value;
			
			UIInputManager im = rctx.InputManager;
			float dragX = 0, dragY = 0;
			
			if (!m_dragMouse)
			{
				bool pr = (m_ti.PressedByTouchId != -1);
				im.TouchHitTest(x - 0.70f * h_width, y - 0.70f * h_height, x + 0.70f * h_width, y + 0.70f * h_height, ref m_ti);
				if (!pr && m_ti.PressedByTouchId != -1)
				{
					// touched.
					m_dragTouch = true;
					m_preDragValue = m_value;
				}
				else if (m_ti.PressedByTouchId == -1)
				{
					m_dragTouch = false;
				}
				if (m_dragTouch)
				{
					dragX = m_ti.CurrentLocation.x - m_ti.PressedLocation.x;
					dragY = m_ti.CurrentLocation.y - m_ti.PressedLocation.y;
				}
			}
			
			/*
			if (!m_dragTouch)
			{
				if (!im.m_state.MouseDown)
				{
					if (im.MouseHitTest(x0, y0, x1, y1))
						im.m_mouseInteraction.ObjectOver = this;
					else if (im.m_mouseInteraction.ObjectOver == this)
						im.m_mouseInteraction.ObjectOver = null;
				}
				
				if (im.m_mouseInteraction.ObjectOver == this)
				{
					if (im.m_state.MouseDown)
					{
						im.m_mouseInteraction.ObjectPressed = this;
						m_dragStartX = im.m_state.MouseX;
						m_dragStartY = im.m_state.MouseY;
						m_preDragValue = m_value;
						m_dragMouse = true;
					}
				}
				
				if (im.m_mouseInteraction.ObjectPressed == this)
				{
					dragX = im.m_state.MouseX - m_dragStartX;
					dragY = im.m_state.MouseY - m_dragStartY;
					im.m_mouseInteraction.ObjectOver = im.MouseHitTest(x0, y0, x1, y1) ? this : null;
					if (!im.m_state.MouseDown)
					{
						im.m_mouseInteraction.ObjectPressed = null;
						m_dragMouse = false;
					}
				}
			}
			*/
			
			if (m_dragMouse || m_dragTouch)
			{
				float w = (layout.x1-layout.x0);
				float h = (layout.y1-layout.y0);
				float deltaDot = dragX * w + dragY * h;
				float delta = deltaDot / (w*w + h*h);
				m_value = m_preDragValue + delta;
				if (m_value < 0.0f)
					m_value = 0.0f;
				else if (m_value > 1.0f)
					m_value = 1.0f;
			}
		}
	}
}
