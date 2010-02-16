## Configured files #########################################################

if(NOT WIN32)
  option(EMACS_MODE "Create and install Emacs mode if Emacs is found" ON)

  if(EMACS_MODE)
    find_program(EMACS NAMES emacs)
    if(NOT EMACS STREQUAL "EMACS-NOTFOUND")
      message(STATUS "WILL install Emacs mode")
      add_custom_command(
        OUTPUT flusspferd.elc
        COMMAND emacs -batch -q -f batch-byte-compile ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
        COMMAND mv ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.elc ${Flusspferd_BINARY_DIR}/
        DEPENDS ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
        COMMENT "Byte compiling Emacs code")
      add_custom_target(
        EMACS_MODE ALL
        DEPENDS flusspferd.elc)
      install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.elc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/emacs/site-lisp)
    endif()

    # install emacs mode source even if emacs is not found
    install(
      FILES ${Flusspferd_SOURCE_DIR}/misc/emacs/flusspferd.el
      DESTINATION ${CMAKE_INSTALL_PREFIX}/share/emacs/site-lisp)
  endif()
endif()
