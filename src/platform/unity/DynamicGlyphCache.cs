using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DynamicGlyphCache
{
	public class Entry
	{
		public DynamicAtlas.TexRect rect;
		public DynamicAtlas atlas;
	}
	
	public struct Cache
	{
		public DynamicAtlas atlas;
	}
	
	public List<Cache> m_atlases = new List<Cache>();
	public Dictionary<string, Entry> m_entries = new Dictionary<string, Entry>();
	
	public DynamicGlyphCache()
	{
	
	}
	
	public Entry GetGlyph(string glyphId, outki.FontGlyphPixData pixData, bool allowAlloc = true)
	{
		if (m_entries.ContainsKey(glyphId))
			return m_entries[glyphId];
			
		for (int i=0;i<m_atlases.Count;i++)
		{
			DynamicAtlas.TexRect r;
			if (m_atlases[i].atlas.Alloc(pixData.pixelWidth, pixData.pixelHeight, out r))
			{
				Color[] col = new Color[pixData.pixelWidth * pixData.pixelHeight];
				for (int j=0;j<pixData.pixelData.Length;j++)
				{
					col[j].a = (float)pixData.pixelData[j] / 255.0f;
					col[j].r = 255.0f;
					col[j].g = 255.0f;
					col[j].b = 255.0f;
				}
				
				m_atlases[i].atlas.SetPixels(r, col);
				Entry e = new Entry();
				e.rect = r;
				e.atlas = m_atlases[i].atlas;
				m_entries[glyphId] = e;
				return e;
			}
		}
		
		if (allowAlloc)
		{
			Cache c = new Cache();
			c.atlas = new DynamicAtlas(512, 512);
			m_atlases.Add(c);
			return GetGlyph(glyphId, pixData, false);
		}
		
		return null;
	}
}

