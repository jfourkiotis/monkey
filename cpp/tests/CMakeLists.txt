include_directories(${CATCH_INCLUDE_DIR} ${COMMON_INCLUDES})

set(TEST_HEADERS)
set(TEST_SOURCES tests_main.cpp)
add_executable(tests ${TEST_HEADERS} ${TEST_SOURCES})
add_dependencies(tests catch)
add_test(NAME TestMonkey COMMAND tests)

