option(PLUGIN_SQLITE3 "Build SQLite3 plugin" ON)

if(PLUGIN_SQLITE3)
  if (WIN32)
    find_library( SQLITE3_LIBRARIES NAMES sqlite3 PATHS "${SQLITE3_LIBRARY_DIRS}")
    if(SQLITE3_LIBRARIES)
      set(SQLITE3_FOUND ON)
      link_directories(${SQLITE3_LIBRARY_DIRS})
      include_directories(${SQLITE3_INCLUDE_DIRS})
    endif()
  else()
    pkg_check_modules(SQLITE3 sqlite3>=3.4.0)
    include_directories(${SQLITE3_INCLUDE_DIRS})
    link_directories(${SQLITE3_LIBRARY_DIRS})
  endif()

  if(FORCE_PLUGINS AND NOT SQLITE3_FOUND)
      message(SEND_ERROR "SQLite3 plugin required but SQLite3 not found")
  elseif(SQLITE3_FOUND)

    if (SQLITE3_STATIC)
        add_library(sqlite3_static STATIC IMPORTED)
        set_property(TARGET sqlite3_static PROPERTY IMPORTED_LOCATION SQLITE3_STATIC)
        set(SQLITE3_LIBRARIES ${SQLITE3_STATIC})
    endif()
  
    flusspferd_plugin(
      "sqlite3"
      DEFINITIONS ${SQLITE3_DEFINITIONS}
      LIBRARIES ${SQLITE3_LIBRARIES} ${Boost_SYSTEM_LIBRARY}
      SOURCES sqlite.cpp
              sqlite_cursor.cpp
              sqlite_cursor.hpp
              sqlite.hpp
              sqlite_plugin.cpp
      JS sqlite3.js)
  endif()

endif()

