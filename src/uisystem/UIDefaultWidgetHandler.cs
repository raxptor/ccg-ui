
namespace CCGUI
{

		// Fallback when no other renderer is available
		public class UIDoNothingRenderer : UIElementRenderer
		{
				public void OnLayout(UIRenderContext rctx, ref UIElementLayout elementLayout)
				{

				}

				public void Render(UIRenderContext rctx, ref UIElementLayout elementLayout)
				{

				}
		}

		public class UIDefaultWidgetHandler
		{
				public static UIWidgetRenderer CreateWidgetRenderer(outki.UIWidget widget, UIWidgetHandler handler)
				{
						// special case for dialogs system.
						if (widget.handler == "dialog-container")
								return new UIDialogContainerRenderer(widget, handler);

						return new UIWidgetRenderer(widget, handler);
				}

				public static UIElementRenderer CreateElementRenderer(outki.UIElement element, UIWidgetHandler handler)
				{
						switch (element._rtti_type)
						{
								case outki.UIWidgetElement.TYPE:
										return handler.CreateWidgetRenderer(((outki.UIWidgetElement)element).widget);
								case outki.UIBitmapElement.TYPE:
										return new UIBitmapElementRenderer((outki.UIBitmapElement)element);
								case outki.UITextElement.TYPE:
										return new UITextElementRenderer((outki.UITextElement)element);
								case outki.UIFillElement.TYPE:
										return new UIFillElementRenderer((outki.UIFillElement)element);
								case outki.UIButtonElement.TYPE:
										return new UIButtonElementRenderer((outki.UIButtonElement)element);
								case outki.UISliderElement.TYPE:
									return new UISliderElementRenderer((outki.UISliderElement)element);
								default:
									return new UIDoNothingRenderer();
						}
				}
		}
}