# Tests CMakeLists.txt
enable_testing()

# Add GoogleTest if not already included
if(NOT TARGET gtest)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src/third_party/googletest)
endif()

# C++ tests
file(GLOB TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/cpp/test_*.cpp)

if(TEST_SOURCES)
    add_executable(test_signal ${TEST_SOURCES})
    target_link_libraries(test_signal PRIVATE gsp gtest gtest_main pthread)
    target_include_directories(test_signal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cpp)
    add_test(NAME test_signal COMMAND test_signal)
endif()

# Python tests (if any)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/python AND BUILD_PYTHON)
    add_test(NAME python_tests
        COMMAND ${Python3_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR}/python
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endif()

# MATLAB tests (if any)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/matlab AND BUILD_MEX_BINDINGS)
    find_package(Matlab)
    if(Matlab_FOUND)
        add_test(NAME matlab_tests
            COMMAND ${Matlab_MAIN_PROGRAM} -sd ${CMAKE_CURRENT_SOURCE_DIR}/..
            -batch "run('${CMAKE_CURRENT_SOURCE_DIR}/matlab/TestMex.m')"
        )
    endif()
endif()
