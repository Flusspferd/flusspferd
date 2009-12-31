option(PLUGIN_XML "Build xml plugin" ON)

if(PLUGIN_XML)
  FIND_PACKAGE(PkgConfig)
  pkg_check_modules(ARABICA arabica)

  if(ARABICA_FOUND)
    if(CMAKE_COMPILER_IS_GNUCXX)
      # These cause a lot of noise from arabica.
      remove_definitions(-Woverloaded-virtual -Wnon-virtual-dtor)
    endif()

    # Use the inlcudes from vendor to, We've made some header only changes
    include_directories( ${CMAKE_SOURCE_DIR}/vendor/arabica/include ${ARABICA_INCLUDEDIR})
    flusspferd_plugin(
      "xml"
      SOURCES attr.cpp
              attr.hpp
              char_data.cpp
              char_data.hpp
              doctype.cpp
              doctype.hpp
              document.cpp
              document.hpp
              dom_exception.cpp
              dom_exception.hpp
              dom_implementation.cpp
              dom_implementation.hpp
              element.cpp
              element.hpp
              misc_nodes.hpp
              named_node_map.cpp
              named_node_map.hpp
              node.cpp
              node.hpp
              node_list.cpp
              node_list.hpp
              node_map.cpp
              node_map.hpp
              parser.cpp
              parser.hpp
              types.hpp
              xml.cpp
      LIBRARIES ${ARABICA_LIBRARIES} )

  elseif(FORCE_PLUGINS)
    message(SEND_ERROR "XML plugin required but arabica not found")
  endif()

endif()
