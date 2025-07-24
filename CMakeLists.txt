cmake_minimum_required(VERSION 3.16)

option(BUILD_PYTHON "export python module" off)
option(BUILD_MEX "export matlab mex file" off)



set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(PyProject)


if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()



set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "" FORCE)

project(${PyProject_NAME} LANGUAGES CXX VERSION ${PyProject_VERSION})
message(STATUS "Project: ${PROJECT_NAME}@v${PROJECT_VERSION}")


# Try to import all Python components potentially needed by nanobind

# find_package(Python3 REQUIRED COMPONENTS Interpreter)





file(GLOB GSP_SOURCES src/sources/*.cpp)
add_library(gsp ${GSP_SOURCES})
add_library(libgsp::libgsp ALIAS gsp)
target_include_directories(gsp
        PUBLIC ${PROJECT_SOURCE_DIR}/include
        PRIVATE ${PROJECT_SOURCE_DIR}/src/sources
)
target_include_directories(gsp PUBLIC src/third_party/spdlog/include)
target_include_directories(gsp PUBLIC src/third_party/mustache)
add_subdirectory(src/third_party/lunasvg)
target_link_libraries(gsp PUBLIC lunasvg::lunasvg)
add_subdirectory(src/third_party/alglib)
target_link_libraries(gsp PUBLIC alglib)



add_executable(example examples/main.cpp)
target_link_libraries(example PUBLIC libgsp::libgsp)
