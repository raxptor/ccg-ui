
	dofile "external/libpng/premake.lua"
	dofile "external/freetype-2.5.3/premake.lua"

	CCGUI_PATH = path.getdirectory(_SCRIPT)

	CCGUI_LIB_INCLUDES = { CCGUI_PATH .. "/src/" }
	CCGUI_RT_INCLUDES = { CCGUI_PATH .. "/src/cpp-runtime" }
	CCGUI_LIB_LINKS = { "ccg-ui-databuilder", "freetype2", "libccgpng" }
	
	function ccgui_use_builder_lib()
		putki_typedefs_runtime("src/types", false, CCGUI_PATH)
		includedirs ( CCGUI_LIB_INCLUDES )
		links { CCGUI_LIB_LINKS }
	end
	
	function ccgui_use_runtime_lib()
		putki_typedefs_runtime("src/types", false, CCGUI_PATH)
		includedirs ( CCGUI_RT_INCLUDES )
                links {"ccg-runtime"}
	end

	project "ccg-ui-databuilder"

		kind "StaticLib"

		language "C++"
		targetname "ccg-ui-databuilder"

		putki_typedefs_builder("src/types", true)
		putki_use_builder_lib()
		
		includedirs { "src" }
		includedirs { "external/libpng"}
		includedirs { "external/freetype-2.5.3/include"}

		files { "src/builder/**.*" }
		files { "src/putki/**.*" }
		files { "src/binpacker/**.*" }

		links { "ccg-ui-putki-lib"}
		links { "freetype2" }
		links { "libccgpng" }

	project "ccg-runtime"

		language "C++"
		targetname "ccg-runtime"
		kind "StaticLib"
		
		includedirs { "src/cpp-runtime" }
                files { "src/cpp-runtime/**.cpp" }
		files { "src/cpp-runtime/**.h" }
		
		putki_use_runtime_lib()
		putki_typedefs_runtime("src/types", true)

  
if os.get() == "windows" then

	project "ccg-ui-csharp"
		kind "SharedLib"
		language "C#"
		targetname "ccg-ui-csharp"

		files { "_gen/outki_csharp/**.cs"}
		files { "_gen/outki_csharp/**.cs"}
		files { "src/uisystem/**.*" }
		files { "src/platform/wpf/**.cs" }
		files { "src/csharp-runtime/**.cs"}

		links { "putki-runtime-csharp" }
		links { "PresentationFramework", "WindowsBase", "PresentationCore", "System.Xaml", "System" }

end
	
