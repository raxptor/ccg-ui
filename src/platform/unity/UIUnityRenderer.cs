using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class UIRenderer
{
	public struct RColor
	{
		public RColor(float _r, float _g, float _b, float _a)
		{
			r = _r;
			g = _g;
			b = _b;
			a = _a;
		}

		public RColor(outki.UIColor col)
		{
			r = col.r / 255.0f;
			g = col.g / 255.0f;
			b = col.b / 255.0f;
			a = col.a / 255.0f;
		}
		
		public float r, g, b, a;
	}
	
	public static RColor m_currentColor = new RColor(1,1,1,1);
	public static int begin = -1;
	public static Shader TexturedShader = null;
	public static Shader SolidShader = null;
	public static Material SolidMaterial = null;
	
	public class LoadedTexture
	{
		public string path;
		public Texture2D unityTexture;
		public Material material;
	};
	
	public class Texture
	{
		public LoadedTexture ld;
		public float u0, v0, u1, v1;
	}
	
	static List<LoadedTexture> loaded = new List<LoadedTexture>();
	
	public static LoadedTexture lastTexture = null;
	
	public static void SetColor(RColor c)
	{
		m_currentColor = c;
	}
	
	public static RColor GetColor()
	{
		return m_currentColor;
	}
	
	public static void MultiplyColor(RColor c)
	{
		m_currentColor.r *= c.r;
		m_currentColor.g *= c.g;
		m_currentColor.b *= c.b;
		m_currentColor.a *= c.a;
	}

	public static Texture ResolveTexture(outki.Texture tex)
	{	
		return ResolveTextureUV(tex, 0, 0, 1, 1);
	}
	
	public static LoadedTexture DynamicTexture(UnityEngine.Texture2D tex)
	{
		LoadedTexture ld = new LoadedTexture();
		ld.unityTexture = tex;
		ld.material = new Material(TexturedShader);
		ld.material.mainTexture = ld.unityTexture;
		return ld;
	}
	
	public static LoadedTexture LoadTexture(outki.Texture tex)
	{
		string imgPath = null;

		outki.TextureOutput to = tex.Output as outki.TextureOutput;
		if (to == null)
			return null;

		outki.DataContainer container = to.Data;
		if (container == null)
			return null;

		outki.DataContainerOutputFile output = container.Output as outki.DataContainerOutputFile;
		if (output == null)
			return null;
	
		imgPath = output.FilePath;
		
		foreach (LoadedTexture lt in loaded)
		{
			if (lt.path == imgPath)
			{
				return lt;
			}
		}

		UnityEngine.Debug.Log("Loading img [" + imgPath + "]");

		LoadedTexture ld = new LoadedTexture();

		TextAsset ta = Resources.Load(imgPath.Replace("Resources/",""), typeof(TextAsset)) as TextAsset;
		if (ta != null)
		{
			ld.unityTexture = new Texture2D (4, 4);
			ld.unityTexture.LoadImage(ta.bytes);
			ld.unityTexture.filterMode = FilterMode.Bilinear;
			ld.unityTexture.anisoLevel = 0;
		}
		else
		{
			// Try loading texture asset
			ld.unityTexture = Resources.Load(imgPath.Replace("Resources/","").Replace(".png", "")) as Texture2D;
			if (ld.unityTexture == null)
			{
				UnityEngine.Debug.LogError("Failed to load texture [" + imgPath + "]");
				return null;
			}
		}

		ld.unityTexture.wrapMode = TextureWrapMode.Clamp;

		ld.material = new Material(TexturedShader);
		ld.material.mainTexture = ld.unityTexture;
		ld.path = imgPath;
		loaded.Add(ld);
		
		UnityEngine.Debug.Log("Loaded texture " + imgPath + " it is "+ ld.unityTexture);		
		return ld;
	}
	
	public static Texture ResolveTextureUV(outki.Texture tex, float u0, float v0, float u1, float v1)
	{
		UIRenderer.Texture t = new Texture();
		t.ld = LoadTexture(tex);

		outki.TextureOutput png = tex.Output as outki.TextureOutput;
		if (png != null)
		{
			// unused for now, need to add for jpeg too?!
			t.u0 = png.u0 + (png.u1 - png.u0) * u0;
			t.v0 = png.v0 + (png.v1 - png.v0) * v0;
			t.u1 = png.u0 + (png.u1 - png.u0) * u1;
			t.v1 = png.v0 + (png.v1 - png.v0) * v1;
		}
		return t;
	}
	
	public static void DrawTexture(Texture tex, float x0, float y0, float x1, float y1)
	{
		DrawTextureUV(tex, x0, y0, x1, y1, tex.u0, tex.v0, tex.u1, tex.v1);
	}

	public static void UseTexture(Texture tex)
	{
		if (tex.ld != lastTexture || begin == 0)
		{
			if (begin != 0)
				GL.End();

			lastTexture = tex.ld;
			lastTexture.material.SetPass(0);
			GL.Begin(GL.QUADS);
			begin = 1;
		}
	}
		
	public static void DrawTextureUV(Texture tex, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1)
	{		
		UseTexture(tex);
		
		GL.Color(new Color(m_currentColor.r, m_currentColor.g, m_currentColor.b, m_currentColor.a));
		GL.TexCoord2(u0, 1 - v0);
		GL.Vertex3(x0, y0, 0);
		GL.TexCoord2(u1, 1 - v0);
		GL.Vertex3(x1, y0, 0);
		GL.TexCoord2(u1, 1 - v1);
		GL.Vertex3(x1, y1, 0);
		GL.TexCoord2(u0, 1 - v1);
		GL.Vertex3(x0, y1, 0);
	}

	public static void DrawTriangleUV(Texture tex, float x0, float y0, float x1, float y1, float x2, float y2, float u0, float v0, float u1, float v1, float u2, float v2)
	{		
		UseTexture(tex);

		GL.Color(new Color(m_currentColor.r, m_currentColor.g, m_currentColor.b, m_currentColor.a));
		GL.TexCoord2(u0, 1 - v0);
		GL.Vertex3(x0, y0, 0);

		GL.TexCoord2(u1, 1 - v1);
		GL.Vertex3(x1, y1, 0);

		GL.TexCoord2(u2, 1 - v2);
		GL.Vertex3(x2, y2, 0);

		// hack
		GL.Vertex3(x2, y2, 0);
	}

	public static void DrawSolidRect(float x0, float y0, float x1, float y1, outki.UIColor col = null)
	{
		if (lastTexture != null || begin == 0)
		{
			if (begin != 0)
				GL.End();
			
			lastTexture = null;
			SolidMaterial.SetPass(0);
			GL.Begin(GL.QUADS);
			begin = 1;
		}

		if (col == null)
			GL.Color(new Color(m_currentColor.r, m_currentColor.g, m_currentColor.b, m_currentColor.a));
		else
			GL.Color(new Color(m_currentColor.r * col.r / 255, m_currentColor.g * col.g / 255.0f, m_currentColor.b * col.b / 255.0f, m_currentColor.a * col.a / 255.0f));

		GL.Vertex3(x0, y0, 0);
		GL.Vertex3(x1, y0, 0);
		GL.Vertex3(x1, y1, 0);
		GL.Vertex3(x0, y1, 0);
	}
	
	public static void HelpGLColor(outki.UIColor t)
	{
		GL.Color(new Color(m_currentColor.r * t.r / 255, m_currentColor.g * t.g / 255.0f, m_currentColor.b * t.b / 255.0f, m_currentColor.a * t.a / 255.0f));		
	}
	
	public static void DrawGradientRect(float x0, float y0, float x1, float y1, 
	                                    outki.UIColor tl, outki.UIColor tr,
	                                    outki.UIColor bl, outki.UIColor br)
	
	{
		if (lastTexture != null || begin == 0)
		{
			if (begin != 0)
				GL.End();
			
			lastTexture = null;
			SolidMaterial.SetPass(0);
			GL.Begin(GL.QUADS);
			begin = 1;
		}

		HelpGLColor(tl);
		GL.Vertex3(x0, y0, 0);
		HelpGLColor(tr);
		GL.Vertex3(x1, y0, 0);
		HelpGLColor(br);
		GL.Vertex3(x1, y1, 0);
		HelpGLColor(bl);
		GL.Vertex3(x0, y1, 0);
	}	

	public static void Begin()
	{
		if (SolidMaterial == null && SolidShader != null)
		{
			SolidMaterial =	new Material(SolidShader);
		}
		
		m_currentColor.r = m_currentColor.g = m_currentColor.b = m_currentColor.a = 1.0f;
		GL.LoadPixelMatrix(0, Screen.width, Screen.height, 0);
	}
	
	public static void End()
	{
		if (begin != 0)
		{
			GL.End();
			begin = 0;
		}
	}
}
