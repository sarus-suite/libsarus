# GTest
if(${ENABLE_UNIT_TESTS})
  add_subdirectory(googletest)
  set(GTEST_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dep/googletest/googletest/include PARENT_SCOPE)
endif(${ENABLE_UNIT_TESTS})
