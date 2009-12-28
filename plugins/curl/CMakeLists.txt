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
        SOURCES exception.hpp
                exception.cpp
                handle_option.hpp
                handle_option.cpp
                get_options.hpp
                get_options.cpp
                integer_option.hpp
                string_option.hpp
                list_option.hpp
                function_option.hpp
                http_post_option.hpp
                http_post_option.cpp
                EasyOpt.hpp
                EasyOpt.cpp
                Easy.hpp
                Easy.cpp
                defines.hpp
                curl_cookie.hpp
                curl_cookie.cpp
                curl.cpp

        LIBRARIES ${CURL_LIBRARIES} ${Boost_SYSTEM_LIBRARY}
      )
    endif()
endif()
