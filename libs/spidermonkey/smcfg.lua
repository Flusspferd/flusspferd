-- vim:ts=4:sts=4:st=4:et:

function spidermonkey_cfg()

    files {
        "src/nanojit/*.cpp",
        "src/nanojit/*.h",
        "src/vprof/*.cpp",
        "src/vproj/*.h",
        "src/*.cpp",
        "src/*.h",
    }

    excludes {
            "src/nanojit/NativeSparc.cpp",
            "src/nanojit/NativePPC.cpp",
            "src/nanojit/NativeARM.cpp",
            "src/nanojit/NativeX64.cpp",
            "src/nanojit/Nativei386.cpp",
            "src/jsdtracef.cpp",
            "src/*gen.cpp",
            "src/jsops.cpp",
    }

    includedirs {
        "src", 
        "include", 
        "include/nanojit", 
        "src/vprof",
        "src/genhdrs",
        "../nspr/include",
    }    

    defines {
        "JS_USE_ONLY_NSPR_LOCKS",
        "HIPPOENV_WIN32",
        "freedtoa=spidermonkey_freedtoa",
--        "JS_THREADSAFE"
--        EXPORT_JS_API
    }           

    configuration "x64"
        defines { "HIPPOENV_X64" }

    configuration "x32"
        defines { "HIPPOENV_X32" }

    configuration "vs*"
        linkoptions{
            "/opt:ref",
            "/opt:noicf",
        }

        buildoptions {
            "-nologo",
            "-W3",
            "-fp:precise",
            "-FIjs-confdefs.h"
        }

    configuration  { "vs*", "Release" }
        buildoptions{
            "-O2",
            "-GL",
        }

    configuration  { "vs*", "Debug" }
        buildoptions{
            "-Zi",
        }

    -- -O3 -fstrict-aliasing
end

