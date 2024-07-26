# CppUTest
if(${ENABLE_UNIT_TESTS})
    set(cpputest_file ${CMAKE_CURRENT_BINARY_DIR}/cpputest/lib/libCppUTest.a)
    add_custom_command(OUTPUT ${cpputest_file} COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/cpputest.sh)
    add_custom_target(cpputest_library DEPENDS ${cpputest_file})
    set(CPPUTEST_LIB ${cpputest_file} PARENT_SCOPE)
    set(CPPUTEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/cpputest/include PARENT_SCOPE)
endif(${ENABLE_UNIT_TESTS})
