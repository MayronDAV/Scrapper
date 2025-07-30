project "lexbor"
	kind "StaticLib"
	language "C"
	staticruntime "off"
	warnings "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

	files
	{
		"lexbor/source/**.h",
		"lexbor/source/**.c",
	}

	removefiles
	{
		"lexbor/source/lexbor/ports/**",
		"lexbor/source/lexbor/core/fs.c",
        "lexbor/source/lexbor/core/memory.c",
        "lexbor/source/lexbor/core/perf.c"
	}

	includedirs
	{
		"lexbor/source",
		"lexbor/source/lexbor"
	}

	defines { "LEXBOR_STATIC=" }

	filter { "system:windows" }
		systemversion "latest"
        files
        {
            "lexbor/source/lexbor/ports/windows_nt/**.c"
        }
        defines { "_CRT_SECURE_NO_WARNINGS" }

    filter { "system:linux or macosx" }
	    pic "on"
		systemversion "latest"

        files
        {
            "lexbor/source/lexbor/ports/posix/**.c"
        }

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"
		optimize "off"

	filter "configurations:Release"
		runtime "Release"
		symbols "on"
		optimize "speed"

	filter "configurations:Dist"
		runtime "Release"
		symbols "off"
		optimize "full"
