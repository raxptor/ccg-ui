
	dofile "external/freetype-2.5.3/premake.lua"

	CCGUI_PATH = path.getdirectory(_SCRIPT)
	CCGUI_LIB_INCLUDES = { CCGUI_PATH .. "/src/" }
	
	function ccgui_use_builder_lib()
		putki_typedefs_builder("src/types", false, CCGUI_PATH)
		includedirs ( CCGUI_LIB_INCLUDES )
		links { "ccg-ui-builder", "freetype2" }
	end
	
	project "ccg-ui-builder"

		kind "StaticLib"

		language "C++"
		targetname "ccg-ui-builder"

		putki_typedefs_builder("src/types", true)
		
		kosmos_use_builder_lib()
		putki_use_builder_lib()
		
		includedirs { "src" }
		includedirs { "external/freetype-2.5.3/include"}

		files { "src/builder/*.cpp" }
		links { "freetype2" }
		links { "libpng" }
