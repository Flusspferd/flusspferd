# vim:ts=2:sw=2:expandtab:autoindent:
#
# The MIT License
#
# Copyright (c) 2008, 2009 Flusspferd contributors (see "CONTRIBUTORS" or
#                                      http://flusspferd.org/contributors.txt)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#

if (APPLE)
  # Enable -flat_namespace so that symbols are resolved transitatively
  SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
      "${CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS} -Wl,-x -flat_namespace")
endif()

# Helper function to make/compile a flusspferd plugin.
# use like:
#
# flusspferd_plugin(
#   xml
#   SOURCES xml.cpp
#   JS xml.js
#   LIBRARIES arabica
#   DEFINITIONS "-Dfoo"
# )
function(flusspferd_plugin PLUGIN)
  # Process named arguments
  #  Creates variabvles PLUGIN_SOURCE etc.
  PARSE_ARGUMENTS("PLUGIN" "SOURCES;JS;LIBRARIES;DEFINITIONS" "" ${ARGN})

  #MESSAGE("*** Arguments for ${PLUGIN}")
  #MESSAGE("Sources: ${PLUGIN_SOURCES}")
  #MESSAGE("JS files: ${PLUGIN_JS}")
  #MESSAGE("Defintions: ${PLUGIN_DEFINITIONS}")
  #MESSAGE("Link libraries: ${PLUGIN_LIBRARIES}")

  if("${PLUGIN_SOURCES}" STREQUAL "")
    message(FATAL_ERROR "flusspferd_plugin called for ${PLUGIN} with no SOURCES argument")
  endif()


  add_library(${PLUGIN}_PLUGIN MODULE ${PLUGIN_SOURCES})

  list(APPEND props
    OUTPUT_NAME ${PLUGIN}
    LIBRARY_OUTPUT_DIRECTORY ${Flusspferd_BINARY_DIR}/modules )

  if(${PLUGIN_DEFINITIONS})
    list(APPEND props DEFINITIONS ${PLUGIN_DEFINITIONS})
  endif()

  if(WIN32)
    # Remove the 'lib' prefix from plugin modules on win32
    list(APPEND props PREFIX "")
  endif()

  set_target_properties( ${PLUGIN}_PLUGIN PROPERTIES ${props} )

  target_link_libraries( ${PLUGIN}_PLUGIN flusspferd ${PLUGIN_LIBRARIES} )

  install( TARGETS ${PLUGIN}_PLUGIN LIBRARY DESTINATION ${INSTALL_MODULES_PATH} )

  if(${PLUGIN}_JS_FILES)
    install( FILES ${PLUGIN_JS} DESTINATION ${INSTALL_MODULES_PATH} )
  endif()

  message(STATUS "Bulding ${PLUGIN} plugin")
endfunction()

# From http://www.cmake.org/Wiki/CMakeMacroParseArguments
MACRO(PARSE_ARGUMENTS prefix arg_names option_names)
  SET(DEFAULT_ARGS)
  FOREACH(arg_name ${arg_names})    
    SET(${prefix}_${arg_name})
  ENDFOREACH(arg_name)
  FOREACH(option ${option_names})
    SET(${prefix}_${option} FALSE)
  ENDFOREACH(option)

  SET(current_arg_name DEFAULT_ARGS)
  SET(current_arg_list)
  FOREACH(arg ${ARGN})            
    SET(larg_names ${arg_names})    
    LIST(FIND larg_names "${arg}" is_arg_name)                   
    IF (is_arg_name GREATER -1)
      SET(${prefix}_${current_arg_name} ${current_arg_list})
      SET(current_arg_name ${arg})
      SET(current_arg_list)
    ELSE (is_arg_name GREATER -1)
      SET(loption_names ${option_names})    
      LIST(FIND loption_names "${arg}" is_option)            
      IF (is_option GREATER -1)
	     SET(${prefix}_${arg} TRUE)
      ELSE (is_option GREATER -1)
	     SET(current_arg_list ${current_arg_list} ${arg})
      ENDIF (is_option GREATER -1)
    ENDIF (is_arg_name GREATER -1)
  ENDFOREACH(arg)
  SET(${prefix}_${current_arg_name} ${current_arg_list})
ENDMACRO(PARSE_ARGUMENTS)

