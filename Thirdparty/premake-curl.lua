project "curl"
    kind "StaticLib"
    language "C"
    staticruntime "off"
    warnings "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/Thirdparty/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/Thirdparty/%{prj.name}")

    files
    {
        "curl/lib/**.c",
        "curl/lib/**.h",
        "curl/include/**.h",
    }

    includedirs
    {
        "curl/include",
        "curl/lib"
    }

    filter "system:windows"
        systemversion "latest"

        links 
        {
            "ws2_32",
            "crypt32",
            "wldap32",
            "advapi32",
            "normaliz"            
        }

        defines 
        {
            "BUILDING_LIBCURL",
            "CURL_STATICLIB",
            "USE_WINDOWS_SSPI",
            "USE_SCHANNEL",
            "USE_WIN32_IDN",
            "HAVE_LDAP_SSL"
        }

    filter { "system:not windows" }
        pic "on"
        systemversion "latest"
        
        defines 
        {
            "BUILDING_LIBCURL",
            "CURL_STATICLIB"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
        optimize "off"

    filter "configurations:Release"
        runtime "Release"
        symbols "on"
        optimize "speed"
        flags { "linktimeoptimization" }

    filter "configurations:Dist"
        runtime "Release"
        symbols "off"
        optimize "full"
        flags { "linktimeoptimization" }
