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

  set_property( TARGET ${PLUGIN}_PLUGIN PROPERTY OUTPUT_NAME ${PLUGIN} )

  if(DEFINED Flusspferd_BINARY_DIR)
    # Only defined in flusspferd build trees
    set_property( TARGET ${PLUGIN}_PLUGIN PROPERTY LIBRARY_OUTPUT_DIRECTORY ${Flusspferd_BINARY_DIR}/modules )
  endif()

  if(WIN32)
    # Remove the 'lib' prefix from plugin modules on win32
    set_target_properties( ${PLUGIN}_PLUGIN PROPERTIES PREFIX "" )
  endif()

  if(DEFINED FLUSSPFERD_PLUGIN_DEFINITIONS)
    list(APPEND PLUGIN_DEFINITIONS ${FLUSSPFERD_PLUGIN_DEFINITIONS})
  endif()

  if(DEFINED PLUGIN_DEFINITIONS)
    # set_target_properties doesn't work when the value is a list itself
    set_property(TARGET ${PLUGIN}_PLUGIN
                 PROPERTY COMPILE_DEFINITIONS ${PLUGIN_DEFINITIONS})
  endif()

  target_link_libraries( ${PLUGIN}_PLUGIN flusspferd ${PLUGIN_LIBRARIES} )
  if(WIN32)
    target_link_libraries( ${PLUGIN}_PLUGIN ${Boost_SYSTEM_LIBRARY} )
  endif()

  install( TARGETS ${PLUGIN}_PLUGIN LIBRARY DESTINATION ${INSTALL_MODULES_PATH} )

  if(${PLUGIN}_JS_FILES)
    install( FILES ${PLUGIN_JS} DESTINATION ${INSTALL_MODULES_PATH} )
  endif()

  message(STATUS "Building ${PLUGIN} plugin")
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

# Read something out of a libtool .la file.
#
# read_libtool("libfoo.la" FOO)
#
# will set FOO_FOUND to FOO_FOUND-NOTFOUND if can't find the file. On success
# FOO_LIBDIR and FOO_LIBRARIES will be populated.
#
# You can call this function with LAFIle being a .a or a .dylib/.so and the
# extension will be changed to .la before reading the file.
#
function(read_libtool LAFILE OUTPUT)

  get_filename_component(ext "${LAFILE}" EXT)
  #message("ext ${ext}")
  if (ext STREQUAL ${CMAKE_SHARED_MODULE_SUFFIX} OR ext STREQUAL ".a")
    get_filename_component(path "${LAFILE}" PATH)
    get_filename_component(namewe "${LAFILE}" NAME_WE)
    set(LAFILE "${path}/${namewe}.la")
  endif()

  mark_as_advanced(${OUTPUT})
  #message("LAFILE ${LAFILE}")

  if(EXISTS "${LAFILE}")
    file(STRINGS "${LAFILE}" FIELD_VALUE REGEX "^dependency_libs='(.*)'$")
    #message("FIELD_VALUE: ${FIELD_VALUE}")
    if(DEFINED FIELD_VALUE)
      STRING(REGEX REPLACE "^dependency_libs='(.*)'$" "\\1" FIELD_VALUE "${FIELD_VALUE}")
      STRING(STRIP FIELD_VALUE "${FIELD_VALUE}")
      #message("FIELD_VALUE: ${FIELD_VALUE}")
      #message("OUTPUT: ${OUTPUT}")
      #message("${OUTPUT}: ${${OUTPUT}}")

      separate_compiler_args(${OUTPUT} "${FIELD_VALUE}" "PARENT_SCOPE")
      #SET(${OUTPUT} "${${OUTPUT}}" PARENT_SCOPE)
      #mark_as_advanced(${OUTPUT})
      return()
    endif()
  endif()

  set(${OUTPUT}_FOUND ${OUTPUT}_FOUND-NOTFOUND)
  #message("OUTPUT: ${OUTPUT} ${${OUTPUT}}")
endfunction()

# Split a string like "-L/usr/local/lib -lfoo" into "${NAME}_LIBRARIES" and "${NAME}_LIBDIR
macro(separate_compiler_args NAME INPUT _parent_scope)
  set(_separate_compiler_args_in "${INPUT}")
  separate_arguments(_separate_compiler_args_in)

  foreach(str ${_separate_compiler_args_in})
    #message("str: " ${str})
    if(str MATCHES "^-L")
      _separate_compiler_args_type("^-L" "${str}" ${NAME}_LIBDIR ${_parent_scope})
    elseif(str MATCHES "^-l")
      _separate_compiler_args_type("^-l" "${str}" ${NAME}_LIBRARIES ${_parent_scope})
    elseif(str MATCHES "^-I")
      _separate_compiler_args_type("^-I" "${str}" ${NAME}_INCLUDEDIR ${_parent_scope})
    elseif(str MATCHES "\\.la$")
      # TODO: This should really recruse and built up _STATIC and non versions
      # read_libtool("${str}" ${NAME})
      string(REGEX REPLACE "\\.la$" ".a" str "${str}")
      _separate_compiler_args_type("" "${str}" ${NAME}_LIBRARIES ${_parent_scope})
    endif()
  endforeach()


  set(_separate_compiler_args_tmp "" CACHE INTERNAL "")
endmacro()

macro(_separate_compiler_args_type _regex _input _output _parent_scope)
  if(NOT "${_regex}" STREQUAL "")
    string(REGEX REPLACE "${_regex}" "" _separate_compiler_args_tmp "${_input}")
  else()
    set(_separate_compiler_args_tmp "${str}")
  endif()
  list(APPEND "${_output}" ${_separate_compiler_args_tmp})

  #message("_separate_compiler_args_type: ${_output} -> ${_separate_compiler_args_tmp}")

  if(NOT ${_parent_scope} STREQUAL "")
    #message("setting ${_output} in PARENT_SCOPE")
    set(${_output} "${${_output}}" PARENT_SCOPE)
  endif()
endmacro()
