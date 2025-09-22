# Tests CMakeLists.txt
enable_testing()
set(CURRENT_SOURCE_DIR ${PROJECT_SOURCE_DIR}/tests)

# Add GoogleTest if not already included
if(NOT TARGET gtest AND NOT TARGET GTest::gtest)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(BUILD_GMOCK  OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory("${CURRENT_SOURCE_DIR}/third_party/googletest")
    set_target_properties(gtest gtest_main PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY  "${CMAKE_BINARY_DIR}"
    )
endif()

# C++ tests
file(GLOB TEST_SOURCES ${CURRENT_SOURCE_DIR}/cpp/test_*.cpp)

if (TEST_SOURCES)
    add_executable(gsp_tests ${TEST_SOURCES})
    target_link_libraries(gsp_tests PRIVATE gsp gtest gtest_main pthread)
    target_include_directories(gsp_tests PRIVATE "${CURRENT_SOURCE_DIR}/cpp")

    add_test(NAME gsp_tests COMMAND gsp_tests)

#    include(GoogleTest)
#    gtest_discover_tests(gsp_tests)
endif()

# Python tests (if any)
if(EXISTS ${CURRENT_SOURCE_DIR}/python AND BUILD_PYTHON)
    add_test(NAME python_tests
        COMMAND ${Python3_EXECUTABLE} -m pytest ${CURRENT_SOURCE_DIR}/python
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# MATLAB tests (if any)
if(EXISTS ${CURRENT_SOURCE_DIR}/matlab AND BUILD_MEX_BINDINGS)
    find_package(Matlab)
    if(Matlab_FOUND)
        add_test(NAME matlab_tests
            COMMAND ${Matlab_MAIN_PROGRAM} -sd ${PROJECT_SOURCE_DIR}
            -batch "run('${CURRENT_SOURCE_DIR}/matlab/TestMex.m')"
        )
    endif()
endif()
