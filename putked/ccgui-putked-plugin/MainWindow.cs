using System;
using Gtk;

public partial class MainWindow: Gtk.Window
{
	public MainWindow (): base (Gtk.WindowType.Toplevel)
	{
		Build ();

		ccguiputkedplugin.WidgetEditor we = new ccguiputkedplugin.WidgetEditor();
		Add(we);
		ShowAll();

	}

	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}
}
