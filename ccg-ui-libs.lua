
	dofile "external/libpng/premake.lua"
	dofile "external/freetype-2.5.0.1/premake.lua"

	CCGUI_PATH = path.getdirectory(_SCRIPT)

	CCGUI_LIB_INCLUDES = { CCGUI_PATH .. "/src/", CCGUI_PATH .. "/_gen" }
	CCGUI_RT_INCLUDES = { CCGUI_PATH .. "/src/cpp-runtime", CCGUI_PATH .. "/_gen" }
	CCGUI_LIB_LINKS = { "freetype2", "libccgpng", "ccg-ui-putki-lib", "ccg-ui-databuilder" }

	project "ccg-ui-putki-lib"

		kind "StaticLib"

		language "C++"
		targetname "ccg-ui-putki-lib"
		
		files { "src/types/**.typedef" }
		files { "_gen/*putki-master.cpp", "_gen/inki/**.h", "_gen/data-dll/**.h" }

		includedirs (PUTKI_LIB_INCLUDES)
		includedirs { "src" }
		includedirs { "_gen" }
		includedirs { "external/libpng" }

		links (PUTKI_LIB_LINKS)

	project "ccg-ui-databuilder"

		kind "StaticLib"

		language "C++"
		targetname "ccg-ui-databuilder"

		includedirs { "_gen" }
		includedirs { "src" }
		includedirs ( PUTKI_LIB_INCLUDES )
		includedirs { "external/libpng"}
		includedirs { "external/freetype-2.5.0.1/include"}

		files { "src/builder/**.*" }
		files { "src/putki/**.*" }
		files { "src/binpacker/**.*" }

		links { "ccg-ui-putki-lib"}
		links { "freetype2" }
		links { "libpng"}

		links (PUTKI_LIB_LINKS)		

	project "ccg-runtime"

		language "C++"
		targetname "ccg-runtime"
		kind "StaticLib"

		includedirs (PUTKI_RT_INCLUDES)
		includedirs { "src/cpp-runtime" }
		includedirs { "_gen" }

                files { "src/cpp-runtime/**.cpp" }
		files { "src/cpp-runtime/**.h" }
		files { "_gen/outki/**.cpp" }
		files { "_gen/outki/**.h" }

		links {"putki-runtime-lib"}

  
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
	
