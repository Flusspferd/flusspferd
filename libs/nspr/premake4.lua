-- vim:ts=4:st=4:sts=4:et:
--

dofile('nsprcfg.lua')

project "nspr"
    kind "SharedLib"
    language "C"
    location "."
    targetname "nspr"
    targetdir(solution().targetdir)

    nspr_cfg()

    configuration "windows"
        links {
            "ws2_32",
            "winmm",
            "mswsock"
        }

project "nspr_static"
    kind "StaticLib"
    language "C"
    location "."
    targetname "libnspr"
    targetdir(solution().targetdir)
    
    excludes {
        "src/md/windows/ntdllmn.c"
    }

    buildoptions {
        "-UNSPR_API -DNSPR_API(x)=x",
        "-UNSPR_DATA_API -DNSPR_DATA_API(x)=x"
    }

    nspr_cfg()


