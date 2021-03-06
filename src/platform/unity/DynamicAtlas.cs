using UnityEngine;
using System.Collections.Generic;

public class DynamicAtlas
{
	public struct TexRect
	{
		public TexRect(int U0, int V0, int U1, int V1)
		{
			u0 = U0; u1 = U1; v0 = V0; v1 = V1;
			_u0 = _v0 = _u1 = _v1 = 0;
		}
		public int u0, u1, v0, v1;
		public float _u0, _v0, _u1, _v1;
	};
	
	public Texture2D m_texture;
	public UIRenderer.Texture m_uitexture;
	private List<TexRect> m_free;
	
	public DynamicAtlas(int width, int height)
	{
		m_texture = new Texture2D(width, height, TextureFormat.ARGB32, false, true);
		m_uitexture = new UIRenderer.Texture();
		m_uitexture.ld = UIRenderer.DynamicTexture(m_texture);
		m_uitexture.u0 = m_uitexture.v0 = 0;
		m_uitexture.u1 = m_uitexture.v1 = 1;
		m_free = new List<TexRect>();
		m_free.Add(new TexRect(0, 0, m_texture.width, m_texture.height));
		Color[] tmp = new Color[width*height];
		for (int i=0;i<tmp.Length;i++)
		{
			tmp[i].r = tmp[i].g = tmp[i].b = 128;
		}
		m_texture.SetPixels(tmp);
	}
	
	public void Clear()
	{
		TexRect f = new TexRect(0, 0, m_texture.width, m_texture.height);
		m_free.Add(f);
	}
	
	public void SetPixels(TexRect where, Color[] array)
	{
		m_texture.SetPixels(where.u0, where.v0, where.u1 - where.u0, where.v1 - where.v0, array, 0);
		m_texture.Apply();
	}
	
	public bool Alloc(int w, int h, out TexRect output)
	{
		int mod = 4;
		int widthRounded = mod * ((w + mod - 1) / mod);
		int padw = widthRounded - w; 
		
		if (AllocReal(w + padw + 2, h + 2, out output))
		{
			float adjw = (1.0f) / (float)m_texture.width;
			float adjh = 1.0f / (float)m_texture.height;
			output._u0 += adjw;
			output._u1 -= adjw + padw / (float)m_texture.width;
			output._v0 -= adjh;
			output._v1 += adjh;
			output.u0 += 1;
			output.v0 += 1;
			output.u1 -= 1 + padw;
			output.v1 -= 1;
			return true;
		}
		return false;
	}
	
	public bool AllocReal(int w, int h, out TexRect output)
	{
		for (int i=0;i<m_free.Count;i++)
		{
			TexRect c = m_free[i];
			int fw = c.u1 - c.u0;
			int fh = c.v1 - c.v0;
			if (fw >= w && fh >= h)
			{
				output = new TexRect(c.u0, c.v0, c.u0 + w, c.v0 + h);
				output._u0 = (float)output.u0 / (float)m_texture.width;
				output._v0 = 1.0f - (float)output.v0 / (float)m_texture.height;
				output._u1 = (float)output.u1 / (float)m_texture.width;
				output._v1 = 1.0f - (float)output.v1 / (float)m_texture.height;
				
				TexRect rhs = new TexRect(c.u0 + w, c.v0, c.u1, c.v1);
				TexRect bottom = new TexRect(c.u0, c.v0 + h, c.u0 + w, c.v1);
				m_free.RemoveAt(i);
				m_free.Add(rhs);
				m_free.Add(bottom);
				return true;
			}
		}
		output = new TexRect(0,0,0,0);
		return false;
	}
	
}
