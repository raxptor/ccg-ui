#include <string>
#include <putki/builder/builder.h>

namespace ccgui
{
	namespace builder { struct data;}
	namespace pngutil
	{
		std::string write_to_temp(putki::builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height);
		std::string write_to_output(putki::builder::data *builder, const char *path, unsigned int *pixbuf, unsigned int width, unsigned int height);

		struct write_buffer
		{
			char *output;
			size_t size;
		};
		
		write_buffer write_to_mem(unsigned int *pixbuf, unsigned int width, unsigned int height);

		struct loaded_png
		{
			unsigned int *pixels;
			unsigned int width, height;
			unsigned int bpp; // always 32 for now
		};

		bool load_info(const char *path, loaded_png *out);
		bool load(const char *path, loaded_png *out);
		void free(loaded_png *png);
	}
}
