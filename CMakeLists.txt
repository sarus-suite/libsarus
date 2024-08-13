cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project("libsarus")

# Set CMake variables
enable_testing()
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
### Required for gcov (e.g., 'example.gcno' instead of 'example.cpp.gcno')
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

message(STATUS "Installation directory: " ${CMAKE_INSTALL_PREFIX})

# Build statically-linked executables
if(${BUILD_STATIC})
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  set(CMAKE_EXE_LINKER_FLAGS "-static")
  set(CMAKE_LINK_SEARCH_START_STATIC TRUE)
  set(CMAKE_LINK_SEARCH_END_STATIC TRUE)
  set(Boost_USE_STATIC_LIBS ON)
endif(${BUILD_STATIC})

# Define CMake variables
set(ENABLE_UNIT_TESTS TRUE CACHE BOOL "Build unit tests. [TRUE]")

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check and set dependencies
find_package(Boost REQUIRED COMPONENTS filesystem regex)
find_package(GTest REQUIRED)
find_program(GIT_PATH git)
### Workaround: issue in linking 'boost::filesystem::detail::copy_file'
add_definitions(-DBOOST_NO_CXX11_SCOPED_ENUMS)

# Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/dep/rapidjson/include)

# Print version
execute_process(COMMAND ${GIT_PATH} describe --tags --dirty --always
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                OUTPUT_VARIABLE LATEST_GIT_TAG
                OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT LATEST_GIT_TAG)
    set(LATEST_GIT_TAG VERSION-NOT-AVAILABLE)
endif(NOT LATEST_GIT_TAG)
message(STATUS "libsarus version: " ${LATEST_GIT_TAG})

# Continue in subdirectories
add_subdirectory(src)
if(${ENABLE_UNIT_TESTS})
  add_subdirectory(test)
endif(${ENABLE_UNIT_TESTS})
