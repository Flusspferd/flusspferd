option(PLUGIN_SUBPROCESS "Build Subprocess plugin" ON)

if(PLUGIN_SUBPROCESS)

  include_directories( ${CMAKE_SOURCE_DIR}/vendor/boost-process )
  flusspferd_plugin(
    "subprocess"
    SOURCES
      exception.cpp
      exception.hpp
      subprocess.cpp
      subprocess.hpp
      subprocess_module.cpp
  )
endif()
