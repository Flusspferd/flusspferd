-- vim:ts=4:sts=4:st=4:et:
project "zlib"
    kind "SharedLib"
    language "C"
    location "." 
    targetname "zdll1"
    implibname "zdll"
    
    files {
            "adler32.c",
            "compress.c",
            "crc32.c",
            "deflate.c",
            "gzio.c",
            "infback.c",
            "inffast.c",
            "inflate.c",
            "inftrees.c",
            "trees.c",
            "uncompr.c",
            "zutil.c"
    }

    flags { "Optimize" }

project "zlib_s"
    kind "StaticLib"
    language "C"
    location "."    
    targetname "z"
    flags { "Optimize" }

    files {
            "adler32.c",
            "compress.c",
            "crc32.c",
            "deflate.c",
            "gzio.c",
            "infback.c",
            "inffast.c",
            "inflate.c",
            "inftrees.c",
            "trees.c",
            "uncompr.c",
            "zutil.c"
    }
    
