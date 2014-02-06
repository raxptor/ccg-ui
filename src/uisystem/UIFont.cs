using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CCGUI
{
	public class UIFont
	{
		outki.Font m_data;

		public struct FormattedGlyph
		{
			public float x, y;
			public float w, h;
			public float u0, v0, u1, v1;
			public bool wordbreak;
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
		
		public UIFont(outki.Font data)
		{
			m_data = data;
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

				if (pen > 0)
				{
					int left = text[i - 1];
					int right = gl;

					for (int k=0;k<f.KerningCharL.Count();k++)
						if (f.KerningCharL[k] == left && f.KerningCharR[k] == right)
						{
							pen += f.KerningOfs[k];
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

				foreach (outki.FontGlyph fgl in f.Glyphs)
				{
					fmt.glyphs[i].w = -666;

					if (fgl.glyph == gl)
					{
						fmt.glyphs[i].u0 = fgl.u0;
						fmt.glyphs[i].v0 = fgl.v0;
						fmt.glyphs[i].u1 = fgl.u1;
						fmt.glyphs[i].v1 = fgl.v1;

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

						pen += fgl.advance;
						break;
					}
				}
			}

			// rescale into possible uncropped texture
			float uo = fmt.texture.u0;
			float us = fmt.texture.u1 - fmt.texture.u0;
			float vo = fmt.texture.v0;
			float vs = fmt.texture.v1 - fmt.texture.v0;

			for (int i = 0; i < text.Length; i++)
			{
				fmt.glyphs[i].u0 = fmt.glyphs[i].u0 * us + uo;
				fmt.glyphs[i].v0 = fmt.glyphs[i].v0 * vs + vo;
				fmt.glyphs[i].u1 = fmt.glyphs[i].u1 * us + uo;
				fmt.glyphs[i].v1 = fmt.glyphs[i].v1 * vs + vo;
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
				UIRenderer.DrawTextureUV(ft.texture, xp, yp, xp + ft.glyphs[i].w, yp + ft.glyphs[i].h, ft.glyphs[i].u0, ft.glyphs[i].v0, ft.glyphs[i].u1, ft.glyphs[i].v1);
			}

			UIRenderer.SetColor(restoreColor);
		}
	}
}
