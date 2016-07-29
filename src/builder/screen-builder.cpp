#include <putki/builder/build.h>
#include <putki/builder/builder.h>
#include <putki/builder/package.h>
#include <putki/builder/build-db.h>

#include <inki/types/ccg-ui/Screen.h>

#include <kosmos-builder-utils/textureconfig.h>

namespace
{
	bool build_screen(const putki::builder::build_info* info)
	{
		inki::ui_screen *screen = (inki::ui_screen *) info->object;
		if (screen->config.get() && screen->config->snap_scale)
		{
			for (int i=0;i<g_outputTexConfigs;i++)
				screen->scaling_for_snapping.push_back(g_outputTexConf[i].scale);
		}
		return true;
	}
};

void register_screen_builder(putki::builder::data *builder)
{
	putki::builder::handler_info info[1] = {
		{ inki::ui_screen::type_id(), "screen-builder-1", build_screen, 0 }
	};
	putki::builder::add_handlers(builder, &info[0], &info[1]);
}