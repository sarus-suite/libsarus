add_subdirectory(aux)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
)
FetchContent_MakeAvailable(googletest)

include(add_unit_test)
set(ADDITIONAL_LINK_LIBS "libsarus_testaux")

add_unit_test("NonRoot" CLIArguments "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" Error "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" Flock "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" HookUtility "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" Lockfile "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" Logger "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" MountParser "${ADDITIONAL_LINK_LIBS}")
add_unit_test("NonRoot" PasswdDB "${ADDITIONAL_LINK_LIBS}")
add_unit_test("Root" DeviceMount "${ADDITIONAL_LINK_LIBS}")
add_unit_test("Root" DeviceParser "${ADDITIONAL_LINK_LIBS}")
add_unit_test("Root" MountUtility "${ADDITIONAL_LINK_LIBS}")
add_unit_test("Root" Mount "${ADDITIONAL_LINK_LIBS}")
add_unit_test("Root" Utility "${ADDITIONAL_LINK_LIBS}")
