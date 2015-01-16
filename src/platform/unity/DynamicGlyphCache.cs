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
	
	public Entry GetGlyph(string glyphId, outki.Font font, outki.FontGlyphPixData pixData, bool allowAlloc = true)
	{
		if (m_entries.ContainsKey(glyphId))
			return m_entries[glyphId];
			
		for (int i=0;i<m_atlases.Count;i++)
		{
			DynamicAtlas.TexRect r;
			if (m_atlases[i].atlas.Alloc(pixData.pixelWidth, pixData.pixelHeight, out r))
			{
				Color[] col = new Color[pixData.pixelWidth * pixData.pixelHeight];
				int[] val = new int[pixData.pixelWidth];
				for (int y=0;y<pixData.pixelHeight;y++)
				{
					int rle0 = pixData.rleDataBegin[y];
					int rle1 = rle0 + pixData.rleDataLength[y];
					int opos = 0;
					for (int j=rle0;j!=rle1;j++)
					{
						if ((font.RLEData[j] & 0x80) == 0x80)
						{
							int count = font.RLEData[j++] & 0x7f;
							byte value = font.RLEData[j];
							for (int b=0;b!=count;b++)
								val[opos++] = value;
						}
						else
						{
							int b = font.RLEData[j] * 2;
							if (b == 254)
								b = 255;
							val[opos++] = b;
						}
					}
					
					for (int x=0;x<pixData.pixelWidth;x++)
					{
						int idx = y * pixData.pixelWidth + x;
						col[idx].a = (float)val[x] / 255.0f;
						col[idx].r = 255.0f;
						col[idx].g = 255.0f;
						col[idx].b = 255.0f;
					}
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
			return GetGlyph(glyphId, font, pixData, false);
		}
		
		return null;
	}
}

