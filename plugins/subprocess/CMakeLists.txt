option(PLUGIN_SUBPROCESS "Build Subprocess plugin" ON)

if(PLUGIN_SUBPROCESS)
  check_cxx_source_runs(
    "#include <unistd.h>
     #include <cstdlib>
     int main()
     {
       return _POSIX_VERSION >= 199506
              ? EXIT_SUCCESS
          : EXIT_FAILURE;
     }"
     FLUSSPFERD_HAVE_POSIX)

   if(FLUSSPFERD_HAVE_POSIX)
     add_definitions(-DHAVE_POSIX)

     set(subprocess_sources
       exception.hpp
       posix/subprocess.hpp
       posix/subprocess.cpp
       posix/errno.hpp
       posix/exception.cpp
       posix/popen.hpp
       posix/popen.cpp
       posix/subprocess_module.cpp)

     flusspferd_plugin(
       "subprocess"
       SOURCES ${subprocess_sources})
   else(FLUSSPFERD_HAVE_POSIX)
     if(FORCE_PLUGINS)
       message(SEND_ERROR "Subprocess plugin not supported on this system")
     else()
       message(STATUS "Subprocess plugin only a stub supported on this system")
     endif()
   endif(FLUSSPFERD_HAVE_POSIX)
endif()
