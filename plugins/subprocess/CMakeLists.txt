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

     check_cxx_source_compiles(
       "#include <sys/epoll.h>
     int main() {
        epoll_create1(0);
     }"
     FLUSSPFERD_HAVE_EPOLL)

   if(FLUSSPFERD_HAVE_EPOLL)
     add_definitions(-DHAVE_EPOLL)
   endif()

   flusspferd_plugin(
     "subprocess"
     SOURCES subprocess.cpp)
 else()
   if(FORCE_PLUGINS)
     message(SEND_ERROR "Subprocess plugin not supported on this system")
   else()
     message(STATUS "Subprocess plugin not supported on this system")
   endif()
 endif()
endif()
