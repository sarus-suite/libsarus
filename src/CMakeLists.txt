file(GLOB libsarus_srcs "*.cpp" "utility/*.cpp")

add_library(libsarus_shared SHARED ${libsarus_srcs})
target_link_libraries(libsarus_shared ${Boost_LIBRARIES})
set_target_properties(libsarus_shared PROPERTIES PREFIX "")
set_target_properties(libsarus_shared PROPERTIES OUTPUT_NAME libsarus CLEAN_DIRECT_OUTPUT 1)

install(TARGETS libsarus_shared DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

add_library(libsarus_static STATIC ${libsarus_srcs})
target_link_libraries(libsarus_static ${Boost_LIBRARIES} pthread dl)
set_target_properties(libsarus_static PROPERTIES PREFIX "")
set_target_properties(libsarus_static PROPERTIES OUTPUT_NAME libsarus CLEAN_DIRECT_OUTPUT 1)

install(TARGETS libsarus_static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
