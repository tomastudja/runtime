set(SOURCES
  EnableEventLogging.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/clralltestevents.cpp
)

set(EVENT_MANIFEST ${VM_DIR}/ClrEtwAll.man)
set(TEST_GENERATOR ${CLR_DIR}/scripts/genEventingTests.py)

include(FindPython)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/clralltestevents.cpp
  COMMAND ${Python_EXECUTABLE} ${TEST_GENERATOR} --testdir "${CMAKE_CURRENT_BINARY_DIR}" --man "${EVENT_MANIFEST}"
  DEPENDS ${EVENT_MANIFEST} ${TEST_GENERATOR}
  COMMENT "Updating clralltestevents.cpp"
)

include_directories(${GENERATED_INCLUDE_DIR})

if(TARGET_UNIX)
    include_directories(${COREPAL_SOURCE_DIR}/inc/rt)
endif(TARGET_UNIX)

add_executable_clr(eventprovidertest
            EXCLUDE_FROM_ALL
            ${SOURCES}
            )
set(EVENT_PROVIDER_DEPENDENCIES "")
if(FEATURE_EVENT_TRACE)
    add_definitions(-DFEATURE_EVENT_TRACE)
    list(APPEND EVENT_PROVIDER_DEPENDENCIES
        eventprovider
        )
    if(CLR_CMAKE_HOST_LINUX)
        list(APPEND EVENT_PROVIDER_DEPENDENCIES
            coreclrtraceptprovider
            )
    endif(CLR_CMAKE_HOST_LINUX)
endif(FEATURE_EVENT_TRACE)

target_link_libraries(eventprovidertest PUBLIC ${EVENT_PROVIDER_DEPENDENCIES} coreclrpal)
add_dependencies(eventprovidertest eventing_headers)

install (TARGETS eventprovidertest DESTINATION paltests/eventprovider COMPONENT paltests EXCLUDE_FROM_ALL)
add_dependencies(paltests_install eventprovidertest)
