option(PLUGIN_SUBPROCESS "Build Subprocess plugin" ON)

if(PLUGIN_SUBPROCESS)

  include_directories( ${CMAKE_SOURCE_DIR}/vendor/boost-process )
  if (WIN32)
    set(libraries "ws2_32")
  endif()
  flusspferd_plugin(
    "subprocess"
    SOURCES
      exception.cpp
      exception.hpp
      subprocess.cpp
      subprocess.hpp
      subprocess_module.cpp
    LIBRARIES ${libraries}
  )
endif()
