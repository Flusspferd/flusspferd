-- vim:ts=4:st=4:sts=4:et:

project "iconv-static"
    kind "StaticLib"
    language "C"
    location "."
    targetdir(solution().targetdir)
    targetname "iconv_static"

    files { "src/win_iconv.c" }        

    configuration "Release"
        flags { "Optimize" }        

project "iconv"
    kind "SharedLib"
    language "C"
    location "."
    targetdir(solution().targetdir)
    targetname "iconv"

    files { "src/win_iconv.c" }
    defines {"HIPPOENV_ICONV_SHARED_BUILD"}
    flags { "Symbols" }

    configuration "Release"
        flags { "Optimize" }        

