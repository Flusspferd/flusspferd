
add_definitions("-DFLUSSPFERD_MODULE_SUFFIX=\"${CMAKE_SHARED_MODULE_SUFFIX}\"")
add_definitions(-DBOOST_FILESYSTEM_NO_DEPRECATED)

## /proc/self/exe ###########################################################

if(RELOCATABLE)
    add_definitions(-DFLUSSPFERD_RELOCATABLE)
    include(FindSelfExe)
    IF(APPLE)
      SET(CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS
          "${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS} -Wl,-x -flat_namespace")
      SET(CMAKE_SHARED_MODULE_CREATE_CXX_FLAGS
          "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -Wl,-x -flat_namespace")
    ENDIF(APPLE)

endif()

## libdl ####################################################################

if(FLUSSPFERD_HAVE_POSIX)
    find_package(DL REQUIRED)
    add_definitions(-DFLUSSPFERD_HAVE_POSIX)
    include_directories(${DL_INCLUDE_DIR})
endif()

## iconv ####################################################################

find_package(Iconv REQUIRED)

if(ICONV_ACCEPTS_NONCONST_INPUT)
    add_definitions(-DICONV_ACCEPTS_NONCONST_INPUT)
endif()

## Spidermonkey #############################################################

set(Spidermonkey_REQUIRED TRUE)
find_package(Spidermonkey)

if(NOT SPIDERMONKEY_FOUND)
    message(FATAL_ERROR "Spidermonkey not found")
endif()

if(SPIDERMONKEY_FOUND)
    if (NOT SPIDERMONKEY_UTF8)
        message(FATAL_ERROR "Spidermonkey UTF8 support required but not found")
    endif()

    if (NOT SPIDERMONKEY_JS_CONFIG_HEADER_PATH)
        message(FATAL_ERROR "Spidermonkey with config.h required but not found (Version 1.8.1+ required)")
    endif()
endif()

include_directories(${SPIDERMONKEY_INCLUDE_DIR})
add_definitions(${SPIDERMONKEY_DEFINITIONS})

## Library ##################################################################

set(
    flusspferd_library_sources
    ../include/flusspferd/aliases.hpp
    ../include/flusspferd/arguments.hpp
    ../include/flusspferd/array.hpp
    ../include/flusspferd/binary.hpp
    ../include/flusspferd/call_context.hpp
    ../include/flusspferd/class_description.hpp
    ../include/flusspferd/class.hpp
    ../include/flusspferd/context.hpp
    ../include/flusspferd/convert.hpp
    ../include/flusspferd/create.hpp
    ../include/flusspferd/create_on.hpp
    ../include/flusspferd/current_context_scope.hpp
    ../include/flusspferd/detail/limit.hpp
    ../include/flusspferd/encodings.hpp
    ../include/flusspferd/evaluate.hpp
    ../include/flusspferd/exception.hpp
    ../include/flusspferd/function_adapter.hpp
    ../include/flusspferd/function.hpp
    ../include/flusspferd/getopt.hpp
    ../include/flusspferd.hpp
    ../include/flusspferd/init.hpp
    ../include/flusspferd/io/binary_stream.hpp
    ../include/flusspferd/io/file.hpp
    ../include/flusspferd/io/filesystem-base.hpp
    ../include/flusspferd/io/io.hpp
    ../include/flusspferd/io/stream.hpp
    ../include/flusspferd/load_core.hpp
    ../include/flusspferd/local_root_scope.hpp
    ../include/flusspferd/modules.hpp
    ../include/flusspferd/native_function_base.hpp
    ../include/flusspferd/native_function.hpp
    ../include/flusspferd/native_object_base.hpp
    ../include/flusspferd/object.hpp
    ../include/flusspferd/properties_functions.hpp
    ../include/flusspferd/property_attributes.hpp
    ../include/flusspferd/property_iterator.hpp
    ../include/flusspferd/root.hpp
    ../include/flusspferd/security.hpp
    ../include/flusspferd/spidermonkey/arguments.hpp
    ../include/flusspferd/spidermonkey/context.hpp
    ../include/flusspferd/spidermonkey/function.hpp
    ../include/flusspferd/spidermonkey/init.hpp
    ../include/flusspferd/spidermonkey/object.hpp
    ../include/flusspferd/spidermonkey/runtime.hpp
    ../include/flusspferd/spidermonkey/string.hpp
    ../include/flusspferd/spidermonkey/value.hpp
    ../include/flusspferd/string.hpp
    ../include/flusspferd/string_io.hpp
    ../include/flusspferd/system.hpp
    ../include/flusspferd/tracer.hpp
    ../include/flusspferd/value.hpp
    ../include/flusspferd/value_io.hpp
    ../include/flusspferd/version.hpp
    binary.cpp
    class.cpp
    convert.cpp
    encodings.cpp
    flusspferd_module.cpp
    function_adapter.cpp
    getopt.cpp
    load_core.cpp
    modules.cpp
    properties_functions.cpp
    property_attributes.cpp
    security.cpp
    system.cpp
    io/binary_stream.cpp
    io/file.cpp
    io/filesystem-base.cpp
    io/io.cpp
    io/stream.cpp
    spidermonkey/arguments.cpp
    spidermonkey/array.cpp
    spidermonkey/context.cpp
    spidermonkey/create.cpp
    spidermonkey/evaluate.cpp
    spidermonkey/exception.cpp
    spidermonkey/function.cpp
    spidermonkey/init.cpp
    spidermonkey/local_root_scope.cpp
    spidermonkey/native_function_base.cpp
    spidermonkey/native_object_base.cpp
    spidermonkey/object.cpp
    spidermonkey/property_iterator.cpp
    spidermonkey/root.cpp
    spidermonkey/string.cpp
    spidermonkey/tracer.cpp
    spidermonkey/value.cpp
)

if(MSVC)
    # Build static lib for MSVC
    set(FLUSSPFERD_LIBRARY_TYPE STATIC)
else()
    set(FLUSSPFERD_LIBRARY_TYPE SHARED)
endif()

add_library(
    flusspferd
    ${FLUSSPFERD_LIBRARY_TYPE}
    ${flusspferd_library_sources})

set(
    flusspferd_LIBS
    ${DL_LIBRARIES}
    ${ICONV_LIBRARIES}
    ${SPIDERMONKEY_LIBRARIES}
    ${Boost_THREAD_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})

target_link_libraries(flusspferd ${flusspferd_LIBS})

# Libraries linking against the flusspferd DSO dont need to link to all the above.
set_property(TARGET flusspferd PROPERTY LINK_INTERFACE_LIBRARIES "")


if(WIN32)
    # .dll goes in bin/
    install(TARGETS flusspferd RUNTIME DESTINATION ${INSTALL_EXECUTABLES_PATH})
    # .lib goes in lib/
    install(TARGETS flusspferd ARCHIVE DESTINATION ${INSTALL_LIBRARIES_PATH})
else()
    install(TARGETS flusspferd LIBRARY DESTINATION ${INSTALL_LIBRARIES_PATH})
endif()

## Library - coverage variant ###############################################

if(ENABLE_TESTS)

# The coverage library has been removed for now. It only makes it less likely
# for people to run the tests, which is a bigger problem than getting coverage
# numbers, which don't work for JS anyways.

#    file(
#        GLOB
#        flusspferd_coverage_extra_sources
#        ${Flusspferd_SOURCE_DIR}/src/io/*.cpp
#        ${Flusspferd_SOURCE_DIR}/src/xml/*.cpp)
#
#    add_library(
#        flusspferd-coverage
#        SHARED
#        ${flusspferd_library_sources} ${flusspferd_coverage_extra_sources})
#
#    target_link_libraries(flusspferd-coverage ${flusspferd_LIBS})
#
#    set(COV_FLAGS "-fprofile-arcs -ftest-coverage")
#
#    set_property(
#        TARGET flusspferd-coverage
#        PROPERTY COMPILE_FLAGS
#        "${COV_FLAGS} -DFLUSSPFERD_COVERAGE")
#
#    set_property(
#        TARGET flusspferd-coverage
#        PROPERTY LINK_FLAGS
#        ${COV_FLAGS})

endif()

## Headers ##################################################################

file(GLOB_RECURSE HEADERFILES *.hpp)

set_source_files_properties(${HEADERFILES}
  PROPERTIES
  HEADER_FILE_ONLY TRUE)

install(
    DIRECTORY include/
    DESTINATION ${INSTALL_HEADERS_PATH}
    FILES_MATCHING PATTERN "*.hpp")

## Configured files #########################################################

if(NOT MSVC)
    set(FLUSSPFERD_EXTERNAL_CFLAGS
        "${FLUSSPFERD_EXTERNAL_CFLAGS} -I${Boost_INCLUDE_DIR} -I${SPIDERMONKEY_INCLUDE_DIR}")

    foreach(_FLAG @SPIDERMONKEY_DEFINITIONS@)
        set(FLUSSPFERD_EXTERNAL_CFLAGS "${FLUSSPFERD_EXTERNAL_CFLAGS} ${_FLAG}")
    endforeach()

    configure_file(
        ${Flusspferd_SOURCE_DIR}/flusspferd.pc.in
        ${Flusspferd_BINARY_DIR}/flusspferd.pc
        @ONLY)

    install(
        FILES ${Flusspferd_BINARY_DIR}/flusspferd.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
endif()
