include_directories(${CPPUTEST_INCLUDE_DIR})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCPPUTEST_MEM_LEAK_DETECTION_DISABLED")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCPPUTEST_MEM_LEAK_DETECTION_DISABLED")

function(add_unit_test test_name test_src_file link_libraries)
    _add_unit_test(${test_name} ${test_src_file} "NotRoot" "${link_libraries}")
endfunction()

function(add_unit_test_as_root test_name test_src_file link_libraries)
    _add_unit_test(${test_name} ${test_src_file} "AsRoot" "${link_libraries}")
endfunction()

function(_add_unit_test test_name test_src_file test_type link_libraries)
    if(${test_type} STREQUAL "AsRoot")
        set(test_name ${test_name}_AsRoot)
    endif()

    set(test_bin_file test_${test_name})
    add_executable(${test_bin_file} ${test_src_file})
    target_link_libraries(${test_bin_file} ${link_libraries} ${CPPUTEST_LIB})
    add_dependencies(${test_bin_file} CPPUTEST_LIBRARY)
    if(${test_type} STREQUAL "AsRoot")
        target_compile_definitions(${test_bin_file} PUBLIC ASROOT)
    endif()

    set(test_commands "cd ${CMAKE_CURRENT_BINARY_DIR} && ${CMAKE_CURRENT_BINARY_DIR}/${test_bin_file}")

    if(${ENABLE_TESTS_WITH_VALGRIND})
        set(test_commands "${test_commands} && valgrind --tool=memcheck --leak-check=full --suppressions=valgrind.suppressions --error-exitcode=1 ${CMAKE_CURRENT_BINARY_DIR}/${test_bin_file}")
    endif()

    add_test(${test_name} bash -c "${test_commands}")
endfunction()

# TODO: above should be removed eventually.

include(GoogleTest)
include_directories(${GTEST_INCLUDE_DIR})
set(TEST_LINK_LIBS "")

function(__gtest_add_unit_test TEST_NAME TEST_SRC_FILE EXEC_AS)
    if(${EXEC_AS} STREQUAL "Root")
        set(TEST_NAME ${TEST_NAME}_AsRoot)
    endif()

    set(TEST_BIN_FILE test_${TEST_NAME})
    add_executable(${TEST_BIN_FILE} ${TEST_SRC_FILE})
    target_link_libraries(${TEST_BIN_FILE} ${TEST_LINK_LIBS} GTest::gtest_main)

    if(${EXEC_AS} STREQUAL "Root")
        target_compile_definitions(${TEST_BIN_FILE} PUBLIC ASROOT)
    endif()

    set(TEST_CMD "cd ${CMAKE_CURRENT_BINARY_DIR} && ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BIN_FILE}")
    add_test(${TEST_NAME} bash -c "${TEST_CMD}")
    #gtest_discover_tests(${TEST_NAME})
endfunction()
