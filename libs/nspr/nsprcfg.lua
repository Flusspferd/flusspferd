-- vim:ts=4:st=4:sts=4:et:
--

function nspr_cfg()            
    defines {
        "_NSPR_BUILD_"
    }

    includedirs {
        "include",
        "include/private"
    }

    files {
        "include/**.h",
        "src/*.c",
        "src/linking/*.c",
        "src/malloc/*.c",
        "src/md/*.c",
        "src/memory/*.c",
        "src/misc/*.c",
        "src/pthreads/*.c",
        "src/io/*.c"
    }

    configuration "vs*"
        defines {
            "_CRT_SECURE_NO_WARNINGS"
        }

        buildoptions {
            "/wd4996", -- No bothering about: [warning C4996: The POSIX name for this item is deprecated.]
            -- The following might be valid but we should not change the NSPR code
            "/wd4018", -- '>=' : signed/unsigned mismatch
            "/wd4554", -- '>>' : check operator precedence for possible error; use parentheses to clarify precedence
            "/wd4244", --  conversion from 'double' to 'PRUint32', possible loss of data
            "/wd4047", -- '==' : 'LONG' differs in levels of indirection from 'PVOID'
            "/wd4024", -- : 'InterlockedCompareExchange' : different types for formal and actual parameter 1
            -- Devel flag for faster compile
            "/MP9"
        }

    configuration "windows"
        defines {
            "HIPPOENV_WIN32",
            "_X86_=1",
--            "_PR_GLOBAL_THREADS_ONLY=1"
        }
        
        files {
            "src/md/windows/*.c",
            "src/threads/**.c"
        }

        excludes {
            "src/md/windows/w95*.c",
            "src/prvrsion.c"
        }

        buildoptions {
            "/UWINNT"
        }
    
    configuration { "not x64", "x32" }
        defines { "HIPPOENV_X32" }

    configuration { "not x32", "x64" }
        defines { "HIPPOENV_X64" }

    configuration { "linux", "not x32", "x64" }
        defines { "x86_64=1" }
        files { "src/md/unix/os_Linux_x86_64.s", scope="config" }
    configuration { "linux", "not x64", "x32" }
        defines { "i386=1" }
        files { "src/md/unix/os_Linux_x86.s", scope="config" }

    configuration "linux"
        defines {
            "HIPPOENV_LINUX",
            "LINUX=1",
            "HAVE_STRERROR=1",
            "HAVE_LCHOWN=1",
            "_REENTRANT=1",
            "FORCE_PR_LOG=1",
            "_PR_PTHREADS=1",
            "HAVE_FCNTL_FILE_LOCKING=1",
            "HAVE_VISIBILITY_PRAGMA=1",
            "HAVE_VISIBILITY_HIDDEN_ATTRIBUTE=1",
            "_GNU_SOURCE=1",
        }

        files {
            "src/threads/prcmon.c",
            "src/threads/prrwlock.c",
            "src/threads/prtpd.c",
            "src/md/unix/unix.c",
            "src/md/unix/unix_errors.c",
            "src/md/unix/uxproces.c",
            "src/md/unix/uxrng.c",
            "src/md/unix/uxshm.c",
            "src/md/unix/uxwrap.c",
            "src/md/unix/linux.c"
        }
        excludes { 
            "src/misc/pripcsem.c",
            "src/io/prdir.c",
            "src/io/prfile.c",
            "src/io/prsocket.c",
            "src/io/prio.c",
        }
        
        buildoptions{
            "-fvisibility=hidden",
            "-pipe",
            "-Wall",
            "-pthread",
            "-fPIC",
            "-UHAVE_CVAR_BUILT_ON_SEM",
        }
end
