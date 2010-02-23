option(PLUGIN_READLINE "Build Readline plugin" ON)

if(PLUGIN_SUBPROCESS)
  include_directories( ${CMAKE_SOURCE_DIR}/vendor/boost-process )

  if(LIBEDIT_FOUND AND NOT LINE_EDITOR STREQUAL "readline")
    set(libraries ${LIBEDIT_LIBRARIES})
  elseif(READLINE_FOUND)
    set(libraries ${READLINE_LIBRARIES})
  endif()

  if (libraries)
    flusspferd_plugin(
      "readline"
      SOURCES readline.cpp
      LIBRARIES ${libraries})
  elseif(FORCE_PLUGINS)
    message(SEND_ERROR "Readline dependencies not fulfilled")
  endif()
endif()
