using System.Collections.Generic;

namespace CCGUI
{
	public interface EventHandler
	{
		void OnEvent(string what);
		bool PollEvent(string what);
	}	
	
	public class UIRenderContext
	{
		// Applied to final coordinates.
		public float LayoutScale;
		public float LayoutOffsetX;
		public float LayoutOffsetY;
		public float FrameDelta;
		public UITextureManager TextureManager;
		public UIInputManager InputManager;
		public EventHandler EventHandler; // app defined
	}
}
