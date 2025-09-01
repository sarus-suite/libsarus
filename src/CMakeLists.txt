file(GLOB libsarus_srcs "*.cpp" "utility/*.cpp")

if(${BUILD_SHARED_LIBS})
  add_library(libsarus SHARED ${libsarus_srcs})
  add_dependencies(libsarus libboost)
  target_link_libraries(libsarus pthread dl $CACHE{LIBBOOST_FILESYSTEM} $CACHE{LIBBOOST_REGEX})
else()
  add_library(libsarus STATIC ${libsarus_srcs})
  add_dependencies(libsarus libboost)
  target_link_libraries(libsarus pthread dl $CACHE{LIBBOOST_FILESYSTEM} $CACHE{LIBBOOST_REGEX})
endif(${BUILD_SHARED_LIBS})

set_target_properties(libsarus PROPERTIES PREFIX "")
set_target_properties(libsarus PROPERTIES OUTPUT_NAME libsarus CLEAN_DIRECT_OUTPUT 1)

install(TARGETS libsarus DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
  GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
