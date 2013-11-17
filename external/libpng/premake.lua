

	project "libccgpng"
		kind "StaticLib"
		language "c"
		targetname "libccgpng"
		files { "*.c", "*.h" }
		excludes { "example.c" }
		includedirs (ZLIB_INCLUDES)
		links {"libz"}
