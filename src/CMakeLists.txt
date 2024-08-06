file(GLOB libsarus_srcs "*.cpp" "utility/*.cpp")
add_library(libsarus SHARED ${libsarus_srcs})
target_link_libraries(libsarus ${Boost_LIBRARIES})
set_target_properties(libsarus PROPERTIES PREFIX "")

install(TARGETS libsarus DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

if(${BUILD_STATIC})
  target_link_libraries(libsarus pthread dl)
endif(${BUILD_STATIC})
