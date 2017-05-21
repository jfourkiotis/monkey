cmake_minimum_required(VERSION 3.0)

project(monkey)

## Catch
include(ExternalProject)
find_package(Git REQUIRED)

ExternalProject_Add(
    catch
    PREFIX ${PROJECT_SOURCE_DIR}/catch
    GIT_REPOSITORY https://github.com/philsquared/Catch.git
    TIMEOUT 10
    UPDATE_COMMAND ${GIT_EXECUTABLE} pull
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
)

ExternalProject_Get_Property(catch source_dir)
set(EXT_PROJECTS_DIR ${PROJECT_SOURCE_DIR}/External)
set(CATCH_INCLUDE_DIR ${source_dir}/include CACHE INTERNAL "${PROJECT_SRC_DIR}/${EXT_PROJECTS_DIR}/catch/")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    if (${CMAKE_SYSTEM_NAME} MATCHES Darwin)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    endif()
endif()

set(HEADERS
    ast_visitor.h
    eval.h
    object.h
    token.h
    lexer.h
    repl.h
    ast.h
    parser.h)
set(SOURCES main.cpp)
add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
add_dependencies(${PROJECT_NAME} catch)

enable_testing(true)
add_subdirectory(tests)

