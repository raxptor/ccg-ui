
// This file has been generated by the GUI designer. Do not modify.
namespace ccguiputkedplugin
{
	public partial class WidgetEditorWidget
	{
		private global::Gtk.VBox m_vbox;
		private global::Gtk.Label m_title;

		protected virtual void Build ()
		{
			global::Stetic.Gui.Initialize (this);
			// Widget ccguiputkedplugin.WidgetEditorWidget
			global::Stetic.BinContainer.Attach (this);
			this.Name = "ccguiputkedplugin.WidgetEditorWidget";
			// Container child ccguiputkedplugin.WidgetEditorWidget.Gtk.Container+ContainerChild
			this.m_vbox = new global::Gtk.VBox ();
			this.m_vbox.Name = "m_vbox";
			this.m_vbox.Spacing = 6;
			// Container child m_vbox.Gtk.Box+BoxChild
			this.m_title = new global::Gtk.Label ();
			this.m_title.Name = "m_title";
			this.m_title.LabelProp = global::Mono.Unix.Catalog.GetString ("label2");
			this.m_vbox.Add (this.m_title);
			global::Gtk.Box.BoxChild w1 = ((global::Gtk.Box.BoxChild)(this.m_vbox [this.m_title]));
			w1.Position = 0;
			w1.Expand = false;
			w1.Fill = false;
			this.Add (this.m_vbox);
			if ((this.Child != null)) {
				this.Child.ShowAll ();
			}
			this.Hide ();
		}
	}
}