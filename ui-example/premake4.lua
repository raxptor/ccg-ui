solution "CCGUI Example project"

	platforms { "x32" }
	configurations {"Release", "Debug"}

	location "build"
	targetdir "build"
	flags { "Symbols" }
	defines {"_CRT_SECURE_NO_WARNINGS"}

	if os.get() == "windows" then
		flags {"StaticRuntime"}
	end

	------------------------------------
	-- Putki must always come first   --
	------------------------------------

	dofile "../../putki/putkilib-premake.lua"

	configuration "Debug"
		defines {"DEBUG"}

	dofile "../ccg-ui-libs.lua"


	project "example-putki-lib"

		language "C++"
		targetname "example-putki-lib"

		if os.get() == "windows" then
			kind "StaticLib"
		else
			kind "SharedLib"
		end

		files { "src/types/**.typedef" }
		files { "_gen/*putki-master.cpp", "_gen/inki/**.h", "_gen/data-dll/**.h" }

		includedirs { "src", "_gen" }
		includedirs ( PUTKI_LIB_INCLUDES )
		includedirs ( CCGUI_LIB_INCLUDES )

		links {"ccg-ui-putki-lib"}
		links {"putki-lib"}

	project "ui-example-databuilder"

		kind "ConsoleApp"
		language "C++"
		targetname "ui-example-databuilder"

		files { "src/putki/builder-main.cpp" }
		files { "src/builder/**.*" }

		includedirs { "src", "_gen" }
		includedirs ( PUTKI_LIB_INCLUDES )
		includedirs ( CCGUI_LIB_INCLUDES )

		links { "example-putki-lib"}
		links { "ccg-ui-databuilder"}
		links { "ccg-ui-putki-lib"}
		links { "putki-lib"}

	project "ui-example-data-dll"

		kind "SharedLib"
		language "C++"
		targetname "ui-example-data-dll"

		files { "src/putki/dll-main.cpp" }

		includedirs { "src", "_gen" }
		includedirs ( PUTKI_LIB_INCLUDES )
		includedirs ( CCGUI_LIB_INCLUDES )

		links { "example-putki-lib"}
		links { "ccg-ui-databuilder"}
		links { "ccg-ui-putki-lib"}
		links { "putki-lib"}

  if os.get() == "windows" then
		
	project "example-ui-csharp"

		kind "WindowedApp"
		language "C#"
		targetname "example-ui-csharp"

		files { "src/viewer/**.cs" }
		files { "_gen/outki_csharp/**.cs" }

		links { "ccg-ui-csharp" }
		links { "putki-runtime-csharp" }
		links { "PresentationFramework", "WindowsBase", "PresentationCore", "System.Xaml", "System" }
	
	end
