function(add_unit_test EXEC_AS TEST_NAME TARGET_LINK_LIBS)
    set(TEST_SRC_FILE test_${TEST_NAME}.cpp)

    if(${EXEC_AS} STREQUAL "Root")
        set(TEST_NAME_PRINT ${TEST_NAME}_AsRoot)
        set(TEST_BIN_FILE test_${TEST_NAME}_AsRoot)
    else()
        set(TEST_NAME_PRINT ${TEST_NAME})
        set(TEST_BIN_FILE test_${TEST_NAME})
    endif()

    add_executable(${TEST_BIN_FILE} ${TEST_SRC_FILE})
    target_link_libraries(${TEST_BIN_FILE} ${TARGET_LINK_LIBS} GTest::gtest_main)

    if(${EXEC_AS} STREQUAL "Root")
        target_compile_definitions(${TEST_BIN_FILE} PUBLIC ASROOT)
    endif()

    set(TEST_CMD "cd ${CMAKE_CURRENT_BINARY_DIR} && ${CMAKE_CURRENT_BINARY_DIR}/${TEST_BIN_FILE}")
    add_test(${TEST_NAME_PRINT} bash -c "${TEST_CMD}")
endfunction()
