cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project("libsarus")

# Set cmake variables
enable_testing()
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
### Required for gcov (e.g., example.gcno instead of example.cpp.gcno)
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

# Enable unit tests (CppUTest)
set(ENABLE_UNIT_TESTS TRUE CACHE BOOL "Build unit tests. Also downloads and builds the CppUTest framework [TRUE]")
set(ENABLE_TESTS_WITH_VALGRIND FALSE CACHE BOOL "Run each unit test through valgrind [FALSE]")

# Check or include dependencies
find_package(Boost REQUIRED COMPONENTS program_options filesystem regex)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/dep/rapidjson/include)
find_program(GIT_PATH git)

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
add_subdirectory(dep)
add_subdirectory(src)

if(${ENABLE_UNIT_TESTS})
  add_subdirectory(test)
endif(${ENABLE_UNIT_TESTS})

# TODO: possibly irrelevant to libsarus?
#install(FILES configure_installation.sh DESTINATION ${CMAKE_INSTALL_PREFIX} PERMISSIONS
#    OWNER_READ OWNER_WRITE OWNER_EXECUTE
#    GROUP_READ GROUP_EXECUTE
#    WORLD_READ WORLD_EXECUTE)
