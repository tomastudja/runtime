project (IjwNativeVarargs)
include(${CLR_ENG_NATIVE_DIR}/ijw/IJW.cmake)

include_directories( ${INC_PLATFORM_DIR} )
set(SOURCES IjwNativeVarargs.cpp)

# add the shared library
add_library (IjwNativeVarargs SHARED ${SOURCES})
target_link_libraries(IjwNativeVarargs PRIVATE ${LINK_LIBRARIES_ADDITIONAL})
add_ijw_msbuild_project_properties(IjwNativeVarargs ijwhost)

# add the install targets
install (TARGETS IjwNativeVarargs DESTINATION bin)
