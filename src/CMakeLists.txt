file(GLOB libsarus_srcs "*.cpp" "utility/*.cpp")
add_library(libsarus SHARED ${libsarus_srcs})
target_link_libraries(libsarus ${Boost_LIBRARIES})
set_target_properties(libsarus PROPERTIES PREFIX "")

install(TARGETS libsarus DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  PERMISSIONS SETUID OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)


#### STUB: Use object library to compile objects only once
## Object library
#add_library(libsarus_objects OBJECT ${libsarus_srcs})
#target_link_libraries(libsarus_objects ${Boost_LIBRARIES})
#set_property(TARGET libsarus_objects PROPERTY POSITION_INDEPENDENT_CODE ON)
#
## Static library
#add_library(libsarus_static STATIC $<TARGET_OBJECTS:libsarus_objects>)
#set_target_properties(libsarus_static PROPERTIES
#    OUTPUT_NAME libsarus
#)
#target_link_libraries(libsarus_static ${Boost_LIBRARIES})
#target_link_libraries(libsarus_static pthread dl)
#set_target_properties(libsarus_static PROPERTIES PREFIX "")
#
#install(TARGETS libsarus_static DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
#
#
## Shared library
#add_library(libsarus_shared SHARED $<TARGET_OBJECTS:libsarus_objects>)
#set_target_properties(libsarus_shared PROPERTIES
#    OUTPUT_NAME sarus
#)
#target_link_libraries(libsarus_shared ${Boost_LIBRARIES})
#
#install(TARGETS libsarus_shared DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
#  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
#    GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
