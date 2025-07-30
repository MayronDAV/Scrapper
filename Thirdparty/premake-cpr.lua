project "cpr"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    warnings "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

    files
    {
        "cpr/cpr/**.cpp",
        "cpr/cpr/**.c",
        "cpr/include/**.h",
        "cpr/include/**.hpp"
    }

    includedirs
    {
        "cpr/include",
        "%{IncludeDir.curl}",
        "%{IncludeDir.libcurl}"
    }

    links { "curl" }
    
    filter "system:linux"
        pic "on"
        systemversion "latest"

    filter "system:macosx"
        pic "on"
        systemversion "latest"

    filter "system:windows"
        systemversion "latest"

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
