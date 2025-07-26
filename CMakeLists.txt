cmake_minimum_required(VERSION 3.16)

option(BUILD_PYTHON_BINDINGS "export python module" OFF)
option(BUILD_MEX_BINDINGS "export matlab mex file" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(BUILD_TESTING "Build tests" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)
option(CMake_BUILD_TYPE "Set build type" "Release")



set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(PyProject)




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




if(DEFINED SKBUILD)
    set(PYTHON_PROJECT_NAME "${SKBUILD_PROJECT_NAME}")
elseif(BUILD_PYTHON)
    set(PYTHON_PROJECT_NAME "${CMAKE_BINARY_DIR}")

    if(NOT PYTHON_REQUIREMENT_INSTALLED)
        execute_process(
                COMMAND "${Python3_EXECUTABLE}" -m pip install
                nanobind ninja pytest # build requirements
                OUTPUT_QUIET
        )
        set(PYTHON_REQUIREMENT_INSTALLED TRUE CACHE INTERNAL "Python requirements installed")
    endif()

    execute_process(
        COMMAND "${Python3_EXECUTABLE}" -m nanobind --cmake_dir
        OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE NB_DIR)

    message(STATUS "Found NanoBind at ${NB_DIR}")
    list(APPEND CMAKE_PREFIX_PATH "${NB_DIR}")
endif()



if(DEFINED PYTHON_PROJECT_NAME)
    # Try to import all Python components potentially needed by nanobind
    #set(Python3_FIND_STRATEGY LOCATION)
    find_package(Python 3.8
            REQUIRED COMPONENTS Interpreter Development.Module
            OPTIONAL_COMPONENTS Development.SABIModule)
    # Import nanobind through CMake's find_package mechanism
    find_package(nanobind CONFIG REQUIRED)


    set(NB_MODULE _about)
    nanobind_add_module(${NB_MODULE} STABLE_ABI NB_STATIC ${PROJECT_SOURCE_DIR}/src/bindings/python/about.cpp)
    target_compile_definitions(${NB_MODULE} PRIVATE
            VERSION_INFO=${PyProject_VERSION_COMPLETE}
            NB_MODULE_NAME=${NB_MODULE})

    file(GLOB PYTHON_BIND_MODULES_PATH ${PROJECT_SOURCE_DIR}/src/bindings/python/bind_*)
    set(PYTHON_BIND_MODULES "${NB_MODULE}")
    foreach(NB_MODULE_PATH IN LISTS PYTHON_BIND_MODULES_PATH)
        get_filename_component(NB_MODULE ${NB_MODULE_PATH} NAME)
        string(REGEX REPLACE "bind_(.*)" "\\1" NB_MODULE ${NB_MODULE})
        list(APPEND PYTHON_BIND_MODULES ${NB_MODULE})
        file(GLOB NB_MODULE_SOURCES ${NB_MODULE_PATH}/*.cpp)

        message(STATUS "Found module: ${NB_MODULE}")

        # Add more commands here to process each directory as needed

        # We are now ready to compile the actual extension module
        nanobind_add_module(
            # Name of the extension
            ${NB_MODULE}

            # Target the stable ABI for Python 3.12+, which reduces
            # the number of binary wheels that must be built. This
            # does nothing on older Python versions
            STABLE_ABI

            # Build libnanobind statically and merge it into the
            # extension (which itself remains a shared library)
            #
            # If your project builds multiple extensions, you can
            # replace this flag by NB_SHARED to conserve space by
            # reusing a shared libnanobind across libraries
            NB_STATIC

            # Source code goes here
            ${NB_MODULE_SOURCES}
        )

        target_include_directories(${NB_MODULE} PRIVATE include src/sources)
        target_link_libraries(${NB_MODULE} PRIVATE libgsp::libgsp)
        target_compile_definitions(${NB_MODULE}  PRIVATE NB_MODULE_NAME=${NB_MODULE})
    endforeach()

    if(DEFINED SKBUILD)
    install(TARGETS ${PYTHON_BIND_MODULES}
            CONFIGURATIONS Release
            LIBRARY DESTINATION
            ${PYTHON_PROJECT_NAME})
    else()
    file(COPY ${PROJECT_SOURCE_DIR}/src/${PROJECT_NAME} DESTINATION ${PYTHON_PROJECT_NAME})

    foreach(_core IN LISTS PYTHON_BIND_MODULES)
        set_target_properties(${_core} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PYTHON_PROJECT_NAME}/${PROJECT_NAME})
    endforeach()
    endif()
endif()

if(DEFINED SKBUILD)
    RETURN()
endif()



if (BUILD_EXAMPLES)
    message(STATUS "Building examples")

      # Add an example executable
   add_executable(example examples/main.cpp)
   target_link_libraries(example PUBLIC libgsp::libgsp)
endif()



