add_definitions(-DCROSS_COMPILE)

if(CLR_CMAKE_HOST_ARCH_AMD64 AND (CLR_CMAKE_TARGET_ARCH_ARM OR CLR_CMAKE_TARGET_ARCH_I386))
    set(FEATURE_CROSSBITNESS 1)
endif(CLR_CMAKE_HOST_ARCH_AMD64 AND (CLR_CMAKE_TARGET_ARCH_ARM OR CLR_CMAKE_TARGET_ARCH_I386))

if (CLR_CMAKE_HOST_OS STREQUAL CLR_CMAKE_TARGET_OS)
    set (CLR_CROSS_COMPONENTS_LIST
        clrjit
        jitinterface_${ARCH_HOST_NAME}
    )

    if(CLR_CMAKE_HOST_LINUX OR NOT FEATURE_CROSSBITNESS)
        list (APPEND CLR_CROSS_COMPONENTS_LIST
            crossgen
        )
    endif()

    if (CLR_CMAKE_TARGET_UNIX)
        list (APPEND CLR_CROSS_COMPONENTS_LIST
            clrjit_unix_${ARCH_TARGET_NAME}_${ARCH_HOST_NAME}
        )
    endif(CLR_CMAKE_TARGET_UNIX)
endif()

if(NOT CLR_CMAKE_HOST_LINUX AND NOT CLR_CMAKE_HOST_OSX AND NOT FEATURE_CROSSBITNESS)
    list (APPEND CLR_CROSS_COMPONENTS_LIST
        mscordaccore
        mscordbi
    )
endif()
