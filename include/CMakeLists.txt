# Install headers (including subdirectories) to /usr/include/libsarus
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    FILES_MATCHING
        PATTERN "*.hpp"
)
