
set(
    LINE_EDITOR
    "libedit"
    CACHE
    STRING
    "Select the line editor (none, libedit, readline)"
)
option(
    FORCE_LINE_EDITOR
    "Fail configuration if the specified line editor is not available"
    OFF)

if(NOT LINE_EDITOR STREQUAL "none" AND
   NOT LINE_EDITOR STREQUAL "libedit" AND
   NOT LINE_EDITOR STREQUAL "readline")
 message(FATAL_ERROR "Invalid option given for LINE_EDTIOR: ${LINE_EDITOR}")
endif()

## libedit ##################################################################

if(LINE_EDITOR STREQUAL "libedit")
  find_package(Libedit)

  if(LIBEDIT_FOUND)
    add_definitions(-DHAVE_EDITLINE)
    include_directories(${LIBEDIT_INCLUDE_DIR})
  endif()

  if(FORCE_LINE_EDITOR AND NOT LIBEDIT_FOUND)
        message(SEND_ERROR "libedit required but it was not found")
  endif()
endif()

## GNU/readline #############################################################

if(LINE_EDITOR STREQUAL "readline")
  message("WARNING: GNU/readline is GPL licensed.")

  find_package(Readline)

  if(READLINE_FOUND)
    add_definitions(-DHAVE_READLINE)
    include_directories(${READLINE_INCLUDE_DIR})
  endif()

  if(FORCE_LINE_EDITOR AND NOT READLINE_FOUND)
        message(SEND_ERROR "readline required but it was not found")
  endif()
endif()

## Programs #################################################################

add_definitions("-DREL_ETC_PATH=\"${REL_ETC_PATH}\""
                "-DREL_LIBDATA_PATH=\"${REL_LIBDATA_PATH}\"")

add_executable(
    flusspferd-shell
    flusspferd.cpp)

set_property(
    TARGET flusspferd-shell
    PROPERTY OUTPUT_NAME
    flusspferd)

set(flusspferd_shell_LIBS flusspferd ${Boost_SYSTEM_LIBRARY})

if(NOT LINE_EDITOR STREQUAL "readline")
  list(APPEND flusspferd_shell_LIBS ${LIBEDIT_LIBRARIES})
else()
  list(APPEND flusspferd_shell_LIBS ${READLINE_LIBRARIES})
endif()

target_link_libraries( flusspferd-shell ${flusspferd_shell_LIBS})

install(TARGETS flusspferd-shell RUNTIME DESTINATION ${INSTALL_EXECUTABLES_PATH})

## Prelude ##################################################################

install( FILES ${Flusspferd_SOURCE_DIR}/js/prelude.js 
         DESTINATION ${INSTALL_LIBDATA_PATH})

## JS Modules ###############################################################

install( DIRECTORY ${Flusspferd_SOURCE_DIR}/js/
         DESTINATION ${INSTALL_MODULES_PATH}
         PATTERN "*.js")

## Configured files #########################################################

configure_file(
    ${Flusspferd_SOURCE_DIR}/js/jsrepl.js.in
    ${Flusspferd_BINARY_DIR}/js/jsrepl.js)

install(
    FILES ${Flusspferd_BINARY_DIR}/js/jsrepl.js
    DESTINATION ${INSTALL_ETC_PATH})

if (NOT WIN32)

    # Man page
    add_custom_command(
      OUTPUT flusspferd.1
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null --hidden-man > ${Flusspferd_BINARY_DIR}/flusspferd.1.options
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null -- ${Flusspferd_SOURCE_DIR}/util/replace-var.js --file-var OPTIONS=${Flusspferd_BINARY_DIR}/flusspferd.1.options ${Flusspferd_SOURCE_DIR}/help/flusspferd.1.in > ${Flusspferd_BINARY_DIR}/flusspferd.1
      DEPENDS ${Flusspferd_SOURCE_DIR}/help/flusspferd.1.in
      DEPENDS flusspferd-shell
      COMMENT "Generating manpage")
    add_custom_command(
      OUTPUT flusspferd.1.gz
      COMMAND gzip -9 -c ${Flusspferd_BINARY_DIR}/flusspferd.1 > ${Flusspferd_BINARY_DIR}/flusspferd.1.gz
      DEPENDS flusspferd.1
      COMMENT "Compressing manpage")
    add_custom_target(
      MANPAGE ALL
      DEPENDS flusspferd.1.gz)
    install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.1.gz
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)

    # Bash completion script
    add_custom_command(
      OUTPUT bash_completion.sh
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null --hidden-bash > ${Flusspferd_BINARY_DIR}/bash_completion.impl
      COMMAND ${Flusspferd_BINARY_DIR}/bin/flusspferd -c /dev/null -- ${Flusspferd_SOURCE_DIR}/util/replace-var.js --file-var IMPL=${Flusspferd_BINARY_DIR}/bash_completion.impl ${Flusspferd_SOURCE_DIR}/misc/bash_completion.sh.in > ${Flusspferd_BINARY_DIR}/bash_completion.sh
      DEPENDS flusspferd-shell
      COMMENT "Generating Bash Completion Script")
    add_custom_target(
      BASH_COMPLETION ALL
      DEPENDS bash_completion.sh)
    install(
      FILES ${Flusspferd_BINARY_DIR}/bash_completion.sh
      DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/bash_completion.d
      RENAME flusspferd)

endif()
