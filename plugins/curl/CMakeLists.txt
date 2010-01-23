option(PLUGIN_CURL "Build cURL plugin" ON)

if(PLUGIN_CURL)

    ## libcurl ##################################################################
    find_package(CURL)

    if(CURL_FOUND)
      set(TMP_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
      set(CMAKE_REQUIRED_LIBRARIES ${CURL_LIBRARIES})

      set(TMP_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
      set(CMAKE_REQUIRED_INCLUDES ${CURL_INCLUDE_DIRS})
      check_cxx_source_runs(
        "#include <curl/curl.h>
         #include <cstdlib>
         int main() {
           return (LIBCURL_VERSION_MAJOR >= 7 && LIBCURL_VERSION_MINOR >= 19) ?
                  EXIT_SUCCESS :
                  EXIT_FAILURE; 
         }"
         CURL_CORRECT_VERSION)
       set(CMAKE_REQUIRED_LIBRARIES ${TMP_CMAKE_REQUIRED_LIBRARIES})
       set(CMAKE_REQUIRED_INCLUDES ${TMP_CMAKE_REQUIRED_INCLUDES})

       if(NOT CURL_CORRECT_VERSION)
         message(STATUS "CURL version too old (7.19.0 or better required) or error during compilation")
         set(CURL_FOUND FALSE)
       endif()
     endif()

    if(FORCE_PLUGINS AND NOT CURL_FOUND)
        message(SEND_ERROR "CURL plugin required but cURL not found")
    elseif(CURL_FOUND)
      include_directories(${CURL_INCLUDE_DIR})
      flusspferd_plugin(
        "curl"
        SOURCES
                Easy.cpp
                Easy.hpp
                EasyOpt.cpp
                EasyOpt.hpp
                curl.cpp
                curl_cookie.cpp
                curl_cookie.hpp
                defines.hpp
                exception.cpp
                exception.hpp
                function_option.hpp
                get_options.cpp
                get_options.hpp
                handle_option.cpp
                handle_option.hpp
                http_post_option.cpp
                http_post_option.hpp
                integer_option.hpp
                list_option.hpp
                string_option.hpp

        LIBRARIES ${CURL_LIBRARIES}
      )
    endif()
endif()
