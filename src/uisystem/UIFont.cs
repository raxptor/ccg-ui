﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CCGUI
{
	public class UIFont
	{
		outki.Font m_data;
		DynamicGlyphCache m_cache;

		public struct FormattedGlyph
		{
			public float x, y;
			public float w, h;
			public float u0, v0, u1, v1;
			public bool wordbreak;
			public UIRenderer.Texture texture;
		}

		public class FormattedText
		{
			public FormattedGlyph[] glyphs;
			public UIRenderer.Texture texture;

			// actual text glyph bouds
			public float x0, y0, x1, y1;
			// used font face's MinY and Maxy
			public float facey0, facey1;

			public int lines;
		};

		float m_spacingMultiplier = 1.0f;
		
		public UIFont(outki.Font data, DynamicGlyphCache cache = null, float spacingMultiplier = 1.0f)
		{
			m_data = data;
			m_cache = cache;
			m_spacingMultiplier = spacingMultiplier;
		}

		public FormattedText FormatText(UIRenderContext ctx, string text, int pixelSize_, float wrapLength = 0.0f)
		{
			// Only during live editing.
			if (m_data == null || pixelSize_ < 1)
				return null;

			FormattedText fmt = new FormattedText();
			fmt.glyphs = new FormattedGlyph[text.Count()];
			fmt.lines = 1;

			float pixelSize = pixelSize_ * ctx.LayoutScale;

			outki.FontOutput f = null;
			double minDiff = 10000;

			foreach (outki.FontOutput fo in m_data.Outputs)
			{
				double diff = (float)Math.Abs(1 - (float)fo.PixelSize / (float)pixelSize);
				if (diff < minDiff)
				{
					minDiff = diff;
					f = fo;
				}
			}

			float scaling = (float)pixelSize / (float)f.PixelSize;

			if (f.OutputTexture != null)
				fmt.texture = ctx.TextureManager.ResolveTexture(f.OutputTexture, 1.0f, 0, 0, 1, 1);

			int pen = 0;
			
			fmt.x0 = 10000;
			fmt.x1 = -10000;

			fmt.y0 = 10000;
			fmt.y1 = -10000;

			fmt.facey0 = - scaling * f.BBoxMaxY / 64.0f;
			fmt.facey1 = - scaling * f.BBoxMinY / 64.0f;

			int penBreak = (int)(wrapLength * 64.0 / scaling);
			float yOffset = 0;

			for (int i = 0; i < text.Length; i++)
			{
				int gl = (int) text[i];

				fmt.glyphs[i].u0 = 0;
				fmt.glyphs[i].v0 = 0;
				fmt.glyphs[i].u1 = 0;
				fmt.glyphs[i].v1 = 0;
				fmt.glyphs[i].wordbreak = (gl == ' ');
				fmt.glyphs[i].w = -666;

				if (pen > 0)
				{
					int left = text[i - 1];
					int right = gl;

					for (int k=0;k<f.KerningCharL.Count();k++)
						if (f.KerningCharL[k] == left && f.KerningCharR[k] == right)
						{
							pen += (int)(m_spacingMultiplier * f.KerningOfs[k]);
							break;
						}
				}

				if (pen > penBreak && penBreak > 0)
				{
					// do word wrap.
					int k = i;
					while (k > 0)
					{
						if (fmt.glyphs[k].wordbreak)
						{
							// leave the wordbreak on that line and start next
							i = k;
							pen = 0;
							yOffset += fmt.facey1 - fmt.facey0;
							break;
						}
						k--;
					}
					// reset to a new line
					if (pen == 0)
					{
						// relayout from here.
						fmt.lines++;
						continue;
					}
				}
				
				bool gotit = false;
				
				foreach (outki.FontGlyphPixData fgl in f.PixGlyphs)
				{
					if (m_cache == null)
						break;

					if (fgl.glyph == gl)
					{
						string glyphId = "fnt-" + f.PixelSize + "-" + fgl.glyph; 
						DynamicGlyphCache.Entry entry = m_cache.GetGlyph(glyphId, m_data, fgl);
						if (entry == null)
							break;

						fmt.glyphs[i].u0 = entry.rect._u0;
						fmt.glyphs[i].v0 = entry.rect._v0;
						fmt.glyphs[i].u1 = entry.rect._u1;
						fmt.glyphs[i].v1 = entry.rect._v1;
						fmt.glyphs[i].texture = entry.atlas.m_uitexture;
						
						float x = (scaling * ((pen + fgl.bearingX) >> 6));
						float y = (scaling * ((0 + fgl.bearingY) >> 6));
						float w = fgl.pixelWidth * scaling;
						float h = fgl.pixelHeight * scaling;
						
						fmt.glyphs[i].x = x;
						fmt.glyphs[i].y = y + yOffset;
						fmt.glyphs[i].w = w;
						fmt.glyphs[i].h = h;
						
						if (x < fmt.x0) fmt.x0 = x;
						if (x+w > fmt.x1) fmt.x1 = x+w;
						if (y < fmt.y0) fmt.y0 = y;
						if (y + h > fmt.y1) fmt.y1 = y + h;
						
						pen += (int)(m_spacingMultiplier * fgl.advance);
						gotit = true;
						break;
					}		
				}
				
				if (gotit)
				{
					continue;
				}
				
				foreach (outki.FontGlyph fgl in f.Glyphs)
				{
					if (fgl.glyph == gl)
					{
						// rescale into possible uncropped texture
						float uo = fmt.texture.u0;
						float us = fmt.texture.u1 - fmt.texture.u0;
						float vo = fmt.texture.v0;
						float vs = fmt.texture.v1 - fmt.texture.v0;
						
						fmt.glyphs[i].u0 = fgl.u0;
						fmt.glyphs[i].v0 = fgl.v0;
						fmt.glyphs[i].u1 = fgl.u1;
						fmt.glyphs[i].v1 = fgl.v1;
						
						fmt.glyphs[i].u0 = fmt.glyphs[i].u0 * us + uo;
						fmt.glyphs[i].v0 = fmt.glyphs[i].v0 * vs + vo;
						fmt.glyphs[i].u1 = fmt.glyphs[i].u1 * us + uo;
						fmt.glyphs[i].v1 = fmt.glyphs[i].v1 * vs + vo;

						float x = (scaling * ((pen + fgl.bearingX) >> 6));
						float y = (scaling * ((0 + fgl.bearingY) >> 6));
						float w = fgl.pixelWidth * scaling;
						float h = fgl.pixelHeight * scaling;
						
						fmt.glyphs[i].x = x;
						fmt.glyphs[i].y = y + yOffset;
						fmt.glyphs[i].w = w;
						fmt.glyphs[i].h = h;
						fmt.glyphs[i].texture = fmt.texture;

						if (x < fmt.x0) fmt.x0 = x;
						if (x+w > fmt.x1) fmt.x1 = x+w;
						if (y < fmt.y0) fmt.y0 = y;
						if (y + h > fmt.y1) fmt.y1 = y + h;

						pen += (int)(m_spacingMultiplier * fgl.advance);
						break;
					}
				}
			}

			return fmt;
		}

		public void Render(UIRenderContext ctx, float x0, float y0, FormattedText ft)
		{
			Render(ctx, x0, y0, ft, new UIRenderer.RColor(1,1,1,1));
		}

		public void Render(UIRenderContext ctx, float x0, float y0, FormattedText ft, UIRenderer.RColor color, int maxGlyphs = -1)
		{
			if (ft == null)
				return;

			UIRenderer.RColor restoreColor = UIRenderer.GetColor();
			UIRenderer.MultiplyColor(color);

			if (maxGlyphs == -1 || maxGlyphs > ft.glyphs.Count())
				maxGlyphs = ft.glyphs.Count();

			for (int i = 0; i < maxGlyphs; i++)
			{
				if (ft.glyphs[i].w == -666)
					continue;
					
				float xp = x0 + ft.glyphs[i].x;
				float yp = y0 + ft.glyphs[i].y;
				UIRenderer.DrawTextureUV(ft.glyphs[i].texture, xp, yp, xp + ft.glyphs[i].w, yp + ft.glyphs[i].h, ft.glyphs[i].u0, ft.glyphs[i].v0, ft.glyphs[i].u1, ft.glyphs[i].v1);
			}

			UIRenderer.SetColor(restoreColor);
		}
	}
}
