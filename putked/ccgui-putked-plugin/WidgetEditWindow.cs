using System;
using System.Collections.Generic;
using PutkEd;
using Gtk;

namespace ccguiputkedplugin
{
	public partial class WidgetEditWindow : Gtk.Window
	{
		inki.UIWidget m_widget;
		AssetEditor m_ae;

		public WidgetEditWindow(DLLLoader.MemInstance mi) : base(Gtk.WindowType.Toplevel)
		{
			this.Build();

			m_widget = DataHelper.CreatePutkEdObj(mi) as inki.UIWidget;

			WidgetEditorWidget wi = new WidgetEditorWidget(mi);
			m_widgetBox.Add(wi);

			if (m_widget.get_layers_size() > 0)
			{
				m_ae = new AssetEditor();

				inki.UIElementLayer layer = m_widget.get_layers(0);
				m_ae.SetObject(layer.m_mi);

				m_propBox.Add(m_ae);
			}

			this.Title = "[" + mi.GetPath() + "] " + mi.GetTypeName() + " editor";


			// Path
			Gtk.CellRendererText r0 = new Gtk.CellRendererText();
			TreeViewColumn c0 = new TreeViewColumn("Item", r0);
			c0.AddAttribute(r0, "text", 0);
			c0.MinWidth = 300;
			m_layers.InsertColumn (c0, 0);

			// Type
			Gtk.CellRendererText r1 = new Gtk.CellRendererText();
			TreeViewColumn c1 = new TreeViewColumn("Action", r1);
			c1.AddAttribute(r1, "text", 1);
			m_layers.InsertColumn (c1, 1);

			TreeStore ts = new TreeStore(typeof(string), typeof(string), typeof(DLLLoader.MemInstance));
			for (int i = 0; i < m_widget.get_layers_size(); i++)
			{
				inki.UIElementLayer layer = m_widget.get_layers(i);
				if (layer == null)
					break;

				TreeIter ti = ts.AppendNode();
				ts.SetValue(ti, 0, "Layer " + (i + 1));

				for (int j = 0; j < layer.get_elements_size(); j++)
				{
					inki.UIElement element = layer.resolve_elements(j);
					if (element == null)
						break;

					TreeIter el = ts.AppendNode(ti);

					ts.SetValue(el, 2, element.m_mi);

					if (element.get_id().Length > 0)
						ts.SetValue(el, 0, element.get_id());
					else
						ts.SetValue(el, 0, "<noname>");
				}
			}
			m_layers.WidthRequest = 300;
			m_layers.Model = ts;

			ShowAll();
		}

		public void LayoutProps()
		{
		}

		protected void OnMLayersCursorChanged (object sender, EventArgs e)
		{
			TreeModel model;
			TreeIter iter;
			if (m_layers.Selection.GetSelected(out model, out iter))
			{		
				DLLLoader.MemInstance mi = (DLLLoader.MemInstance) model.GetValue(iter, 2);
				if (mi == null)
				{
					m_ae.Visible = false;
				}
				else
				{
					m_ae.Visible = true;
					m_ae.SetObject(mi);
				}
			}
		}
	}
}

