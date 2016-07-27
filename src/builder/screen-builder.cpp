#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/resource.h>
#include <putki/builder/build-db.h>
#include <putki/builder/db.h>

#include <inki/types/ccg-ui/Screen.h>

#include <kosmos-builder-utils/textureconfig.h>

struct screenbuilder : putki::builder::handler_i
{
	virtual const char *version() {
		return "screen-builder-1";
	}

	virtual bool handle(putki::builder::build_context *context, putki::builder::data *builder, putki::build_db::record *record, putki::db::data *input, const char *path, putki::instance_t obj)
	{
		inki::ui_screen *screen = (inki::ui_screen *) obj;

		// ERR IF screen->Config == null

		if (screen && screen->config && screen->config->snap_scale)
		{
			for (int i=0;i<g_outputTexConfigs;i++)
				screen->scaling_for_snapping.push_back(g_outputTexConf[i].scale);
		}

		return false;
	}
};

void register_screen_builder(putki::builder::data *builder)
{
	static screenbuilder fb;
	putki::builder::add_data_builder(builder, "UIScreen", &fb);
}
