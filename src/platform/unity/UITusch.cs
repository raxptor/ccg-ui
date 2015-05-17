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

		if (Input.GetMouseButton(0))
		{
			Tch t = new Tch();
			t.position.x = Input.mousePosition.x;
			t.position.y = Screen.height - Input.mousePosition.y;
			t.phase = !m_mouseWasDown ? TouchPhase.Began : TouchPhase.Moved;
			t.fingerId = 1;
			Tch[] u = new Tch[1];
			u[0] = t;
			m_mouseWasDown = true;
			return u;
		}
		else
		{
			m_mouseWasDown = false;
			return new Tch[0];
		}
	}
}

