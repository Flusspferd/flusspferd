## Tests ###################################################################

if(ENABLE_TESTS)
    set(
      TESTS
      test_arguments.cpp
      test_array.cpp
      test_class_description.cpp
      test_context.cpp
      test_create.cpp
      test_current_context_scope.cpp
      test_evaluate.cpp
      test_exception.cpp
      test_function.cpp
      test_init.cpp
      test_io.cpp
      test_object.cpp
      test_property_iterator.cpp
      test_regression_159.cpp
      test_string.cpp
      test_value.cpp
    )

    foreach(TEST_SOURCE ${TESTS})
        string(REGEX MATCH "test_[a-zA-Z0-9_]*" TEST_OUTPUT ${TEST_SOURCE})
        string(REGEX REPLACE "^test_" "" TEST_NAME ${TEST_OUTPUT})
        set(TEST_SOURCES ${TEST_SOURCE} test_environment.hpp)
        add_executable(${TEST_OUTPUT} ${TEST_SOURCES})
        set_property(
            TARGET ${TEST_OUTPUT}
            PROPERTY COMPILE_FLAGS
            "-DBOOST_TEST_DYN_LINK -DBOOST_TEST_MODULE=${TEST_NAME}")
        target_link_libraries(
            ${TEST_OUTPUT}
            ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
            flusspferd)
    endforeach()

endif()
