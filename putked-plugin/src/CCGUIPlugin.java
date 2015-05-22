import javafx.scene.Node;
import javafx.scene.control.ColorPicker;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Priority;
import javafx.scene.paint.Color;
import inki.*;
import putked.*;
import putked.Interop.Field;
import putked.Interop.MemInstance;

class ColorEditor implements putked.FieldEditor
{
	CCGUI.UIColor m_color;
	Field m_field;
	
	public ColorEditor(CCGUI.UIColor color, Field f) {
		m_color = color;
        m_field = f;
	}
	
	private long d2b(double b) {
		return (long)(b/255.0f);
	}

	@Override
	public Node createUI() {
        HBox hb = new HBox();
        hb.setMaxWidth(Double.MAX_VALUE);
        
        ColorPicker ed = new ColorPicker();
        ed.setValue(new Color(m_color.get_r()/255.0f, m_color.get_g()/255.0f, m_color.get_b()/255.0f, m_color.get_a()/255.0f));
        ed.valueProperty().addListener( (a, prev, next) -> {
        	m_color.set_r(d2b(next.getRed()));;
        	m_color.set_g(d2b(next.getGreen()));
        	m_color.set_b(d2b(next.getBlue()));
        	m_color.set_a(d2b(next.getOpacity()));
        });
        
        if (m_field.isArray())
        	hb.getChildren().setAll(ed);
        else
           	hb.getChildren().setAll(putked.EditorCreatorUtil.makeLabel(m_field, 0), ed);
   	
        HBox.setHgrow(ed, Priority.ALWAYS);
		return hb;
	}
}

class FieldCreator implements putked.FieldEditorCreator
{

	@Override
	public FieldEditor createEditor(MemInstance mi, Field field, int index, boolean asArray) 
	{
		if (asArray) {
			return null;
		}
		if (field.getType() == Interop.FT_STRUCT_INSTANCE && field.getRefType().equals(CCGUI.UIColor.NAME)) {
			field.setArrayIndex(index);
            MemInstance _mi = field.getStructInstance(mi);
            CCGUI.UIColor color = new CCGUI.UIColor();
            color.connect(_mi);
        	return new ColorEditor(color, field);
		}
		return null;
	}
	
}

public class CCGUIPlugin implements putked.EditorPluginDescription
{
	@Override
	public String getName() { return "CCGUIPlugin"; }
	
	@Override
	public String getVersion() { return "1.0"; }

	@Override
	public PluginType getType() { return PluginType.PLUGIN_EDITOR; }

	@Override
	public void start()
	{ 
		DataHelper.addTypeService(new CCGUI());
		putked.ObjectEditor.addCustomTypeEditorCreator(new FieldCreator());
	}
}
