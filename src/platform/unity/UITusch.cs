using System;
using UnityEngine;

public class UITusch
{
	public class Tch
	{
		public TouchPhase phase;
		public Vector3 position;
		public int fingerId;
	};
	
	public UITusch()
	{
		
	}
	
	static bool m_mouseWasDown;
	static Tch[] m_mouseEmu = new Tch[0];
	
	public static void UpdateMouseEmu()
	{
		if (Input.GetMouseButton(0))
		{
			Tch t = new Tch();
			t.position.x = Input.mousePosition.x;
			t.position.y = Screen.height - Input.mousePosition.y;
			t.phase = !m_mouseWasDown ? TouchPhase.Began : TouchPhase.Moved;
			t.fingerId = 1;
			m_mouseEmu = new Tch[1];
			m_mouseEmu[0] = t;
			m_mouseWasDown = true;
		}
		else
		{
			m_mouseWasDown = false;
			m_mouseEmu = new Tch[0];
		}
	}
	
	static public Tch[] Read()
	{
		if (!UnityEngine.Application.isEditor)
		{
			Tch[] n = new Tch[Input.touches.Length];
			for (int i=0;i<Input.touches.Length;i++)
			{
				n[i] = new Tch();
				n[i].position = Input.touches[i].position;
				n[i].position.y = Screen.height - n[i].position.y;
				n[i].fingerId = Input.touches[i].fingerId;
				n[i].phase = Input.touches[i].phase;
			}
			return n;
		}

		return m_mouseEmu;
	}
}

