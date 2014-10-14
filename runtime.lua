
	CCGUI_RT_INCLUDES = { CCGUI_PATH .. "/src/cpp-runtime" }

	function ccgui_use_runtime_lib()
		putki_typedefs_runtime("src/types", false, CCGUI_PATH)
		includedirs ( CCGUI_RT_INCLUDES )
                links {"ccg-runtime"}
	end

	project "ccg-runtime"

		language "C++"
		targetname "ccg-runtime"
		kind "StaticLib"
		
		includedirs { "src/cpp-runtime" }
                files { "src/cpp-runtime/**.cpp" }
		files { "src/cpp-runtime/**.h" }
		
		putki_use_runtime_lib()
		kosmos_use_runtime_lib()
		
		putki_typedefs_runtime("src/types", true)


--[[
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
]]--
