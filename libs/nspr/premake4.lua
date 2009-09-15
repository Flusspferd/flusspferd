-- vim:ts=4:st=4:sts=4:et:
--

dofile('nsprcfg.lua')

project "nspr"
    kind "SharedLib"
    language "C"
    location "."
    targetname "nspr"

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
    
    excludes {
        "src/md/windows/ntdllmn.c"
    }

    nspr_cfg()


