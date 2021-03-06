namespace CCGUI
{
	class UITextElementRenderer : UIElementRenderer
	{
		outki.UITextElement m_element;
		UIFont m_font;
		UIFont.FormattedText m_fmted;
		float m_x0, m_y0;

		public UITextElementRenderer(outki.UITextElement element)
		{
			m_element = element;
			m_font = new UIFont(m_element.font);
		}

		public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
		{
			float wrapLength = 0;
			if (m_element.WordWrap)
				wrapLength = elementLayout.x1 - elementLayout.x0;
		
			m_fmted = m_font.FormatText(rctx, m_element.Text, m_element.pixelSize, wrapLength);
			if (m_fmted == null)
				return;

			switch (m_element.HorizontalAlignment)
			{
				case outki.UIHorizontalAlignment.UIHorizontalAlignment_Center:
					m_x0 = (elementLayout.x0 + elementLayout.x1 - (m_fmted.x1 - m_fmted.x0)) / 2 - m_fmted.x0;
					break;
				case outki.UIHorizontalAlignment.UIHorizontalAlignment_Right:
					m_x0 = elementLayout.x1 - m_fmted.x1;
					break;
				default: // left align
					m_x0 = elementLayout.x0 - m_fmted.x0;
					break;
			}

			switch (m_element.VerticalAlignment)
			{
				case outki.UIVerticalAlignment.UIVerticalAlignment_Top:
					m_y0 = elementLayout.y0 - m_fmted.facey0;
					break;
				case outki.UIVerticalAlignment.UIVerticalAlignment_Bottom:
					m_y0 = elementLayout.y1 - m_fmted.facey1;
					break;
				default: // center
					// glyph actual sizes method
					m_y0 = (elementLayout.y0 + elementLayout.y1 - (m_fmted.y1 - m_fmted.y0)) / 2 - m_fmted.y0;
					break;
			}

		}

		public void Render(UIRenderContext rctx, ref UIElementLayout layout)
		{
			UIRenderer.RColor rc, old;
			old = UIRenderer.GetColor();
			rc.r = (float)m_element.color.r / 255.0f;
			rc.g = (float)m_element.color.g / 255.0f;
			rc.b = (float)m_element.color.b / 255.0f;
			rc.a = (float)m_element.color.a / 255.0f;
			UIRenderer.SetColor(rc);
			m_font.Render(rctx, m_x0, m_y0, m_fmted);
			UIRenderer.SetColor(old);
		}
	}
}