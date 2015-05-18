using System;
using System.Collections.Generic;
using System.Text;

namespace CCGUI
{
	public struct MouseInteraction
	{
		public object ObjectOver;
		public object ObjectPressed;
	};
	
	public class UITouchInteraction
	{
		public UITouchInteraction()
		{
			PressedByTouchId = -1;
			StillInside = false;
		}
		
		public bool StillInside;
		public int PressedByTouchId;
		public UnityEngine.Vector3 PressedLocation, CurrentLocation;
	}

	public class UIInputState
	{
		public float MouseX;
		public float MouseY;
		public bool MouseDown, MouseClicked;
		
		public UITusch.Tch[] Touches;
	};

	public class UIInputManager
	{
		public UIInputState m_state = null;
		public MouseInteraction m_mouseInteraction = new MouseInteraction();

		private int m_depth;
		private int m_block;

		public UIInputManager()
		{

		}

		public void BeginFrame(UIInputState state)
		{
			m_state = state;
			m_depth = 0;
			m_block = 0;
		}

		public void EndFrame()
		{

		}

		public void EnterLayer()
		{
			m_depth++;
		}

		public void LeaveLayer()
		{
			m_depth--;
		}
		
		public void HackBlockInput(int refcount)
		{
			m_block += refcount;
		}

		public bool MouseHitTest(float x0, float y0, float x1, float y1)
		{
			if (m_block != 0)
				return false;
			
			return InputHitTest(m_state.MouseX, m_state.MouseY, x0, y0, x1, y1);
		}

		public bool InputHitTest(float x, float y, float x0, float y0, float x1, float y1)
		{
			if (m_block != 0)
				return false;
				
			return x >= x0 && y >= y0 && x < x1 && y < y1;
		}
		
		public void TouchHitTest(float x0, float y0, float x1, float y1, ref UITouchInteraction interaction)
		{
			if (m_block == 0)
			{
				foreach (UITusch.Tch t in m_state.Touches)
				{
					// if the touch we track appear again, reset everything...
					if (t.phase == UnityEngine.TouchPhase.Began && interaction.PressedByTouchId == t.fingerId)
						interaction.PressedByTouchId = -1;
						
					if (t.position.x >= x0 && t.position.y >= y0 && t.position.x < x1 && t.position.y < y1)
					{
						if (t.phase == UnityEngine.TouchPhase.Began)
						{
							if (interaction.PressedByTouchId == -1)
							{
								interaction.PressedLocation = t.position;
								interaction.PressedByTouchId = t.fingerId;
							}
						}
						
						if (interaction.PressedByTouchId == t.fingerId)
						{
							interaction.CurrentLocation = t.position;
							interaction.StillInside = true;
						}
						return;
					}
					else if (t.fingerId == interaction.PressedByTouchId)
					{
						interaction.CurrentLocation = t.position;
						interaction.StillInside = false;
						return;
					}
				}
			}
			
			interaction.PressedByTouchId = -1;
			interaction.StillInside = false;
		}
	}
}
