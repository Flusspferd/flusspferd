option(PLUGIN_GMP "Build GMP plugin" ON)

if(PLUGIN_GMP)
  find_package(GMP)
  find_package(GMPXX)

  if(FORCE_PLUGINS)
    if(NOT GMP_FOUND)
      message(SEND_ERROR "GMP plugin required but GMP/GMPXX not found")
    endif()
  elseif(GMP_FOUND AND GMPXX_FOUND)
    include_directories(${GMP_INCLUDE_DIR})
    flusspferd_plugin(
      "gmp"
      SOURCES exception.hpp
              Float.cpp
              Float.hpp
              gmp.cpp
              Integer.cpp
              Integer.hpp
              Rational.cpp
              Rational.hpp
      LIBRARIES ${GMPXX_LIBRARY} ${GMP_LIBRARY})
  endif()
endif()

