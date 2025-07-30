include "dependencies.lua"

workspace "Scrapper"
    architecture "x64"
    startproject "Scrapper"
    flags "MultiProcessorCompile"
    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
    
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    IMGUI_GLFW 		= "ON"
    IMGUI_OPENGL 	= "ON"

    filter "system:windows"
        systemversion "latest"
    
    filter { "system:windows", "configurations:Dist" }
        linkoptions { "/SUBSYSTEM:WINDOWS" }

group "Thirdparty"
    include "Tools"
    include "Thirdparty/glad"
    include "Thirdparty/premake-glfw.lua"
    include "Thirdparty/premake-imgui.lua"
    include "Thirdparty/premake-curl.lua"
    include "Thirdparty/premake-cpr.lua"
    include "Thirdparty/premake-lexbor.lua"
group ""

group "Core"
    project "Scrapper"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"
        staticruntime "On"

        targetdir ("bin/" .. outputdir .. "/%{prj.name}")
        objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

        files
        {
            "Source/**.h",
            "Source/**.cpp"
        }

        includedirs
        {
            "Source",
            "%{IncludeDir.glad}",
            "%{IncludeDir.glfw}",
            "%{IncludeDir.imgui}",
            "%{IncludeDir.curl}",
            "%{IncludeDir.libcurl}",
            "%{IncludeDir.cpr}",
            "%{IncludeDir.lexbor}"
        }

        links
        {
            "glad",
            "glfw",
            "imgui",
            "curl",
            "cpr",
            "lexbor",
        }

        filter "system:windows"
            systemversion "latest"
            buildoptions { "/utf-8", "/Zc:char8_t-", "/wd4251", "/wd4275" }

            links "opengl32.lib"

            defines
            {
                "_WINDOWS",
                "GLFW_INCLUDE_NONE",
                "UNICODE", 
                "_UNICODE"
            }

        filter "system:linux"
            pic "on"
            links "GL"
            buildoptions { "`pkg-config --cflags gtk+-3.0`", "-finput-charset=UTF-8", "-fexec-charset=UTF-8", "-fno-char8_t", "-Wno-effc++", "-fpermissive" }
            linkoptions { "`pkg-config --libs gtk+-3.0`" }
            
            defines
            {
                "_LINUX",
                "GLFW_INCLUDE_NONE"
            }

        filter "configurations:Debug"
            defines "_DEBUG"
            runtime "Debug"
            symbols "on"
            optimize "off"

        filter "configurations:Release"
            defines "_RELEASE"
            runtime "Release"
            symbols "on"
            optimize "speed"

        filter "configurations:Dist"
            defines "_DIST"
            runtime "Release"
            symbols "off"
            optimize "full"
group ""