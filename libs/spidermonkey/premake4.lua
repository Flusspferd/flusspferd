-- vim:ts=4:sts=4:st=4:et:

dofile("smcfg.lua")

project "spidermonkey-static"
    kind "StaticLib"
    language "C++"
    location(".")
    targetdir(solution().targetdir)

    defines { "EXPORT_JS_API" }
    spidermonkey_cfg()

project "spidermonkey"
    kind "SharedLib"
    language "C++"
    location "."
    targetdir(solution().targetdir)

    links { "winmm", "nspr_static" }
    defines { "EXPORT_JS_API" }

    spidermonkey_cfg()

    configuration "vs*"    
        buildoptions {
            "-UNSPR_API -DNSPR_API(x)=x",
            "-UNSPR_DATA_API -DNSPR_DATA_API(x)=x"
        }

