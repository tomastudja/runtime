# Set initial flags for each configuration

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0083 NEW)

include(CheckCXXCompilerFlag)

# "configureoptimization.cmake" must be included after CLR_CMAKE_HOST_UNIX has been set.
include(${CMAKE_CURRENT_LIST_DIR}/configureoptimization.cmake)

#-----------------------------------------------------
# Initialize Cmake compiler flags and other variables
#-----------------------------------------------------

if(MSVC)
    add_compile_options(/Zi /FC /Zc:strictStrings)
elseif (CLR_CMAKE_HOST_UNIX)
    add_compile_options(-g)
    add_compile_options(-Wall)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        add_compile_options(-Wno-null-conversion)
    else()
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror=conversion-null>)
    endif()
endif()

if (CMAKE_CONFIGURATION_TYPES) # multi-configuration generator?
    set(CMAKE_CONFIGURATION_TYPES "Debug;Checked;Release;RelWithDebInfo" CACHE STRING "" FORCE)
endif (CMAKE_CONFIGURATION_TYPES)

set(CMAKE_C_FLAGS_CHECKED "")
set(CMAKE_CXX_FLAGS_CHECKED "")
set(CMAKE_EXE_LINKER_FLAGS_CHECKED "")
set(CMAKE_SHARED_LINKER_FLAGS_CHECKED "")

add_compile_definitions("$<$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>>:DEBUG;_DEBUG;_DBG;URTBLDENV_FRIENDLY=Checked;BUILDENV_CHECKED=1>")
add_compile_definitions("$<$<OR:$<CONFIG:RELEASE>,$<CONFIG:RELWITHDEBINFO>>:NDEBUG;URTBLDENV_FRIENDLY=Retail>")

set(CMAKE_CXX_STANDARD_LIBRARIES "") # do not link against standard win32 libs i.e. kernel32, uuid, user32, etc.

if (MSVC)
  add_link_options(/GUARD:CF)

  # Linker flags
  #
  set (WINDOWS_SUBSYSTEM_VERSION 6.01)

  if (CLR_CMAKE_HOST_ARCH_ARM)
    set(WINDOWS_SUBSYSTEM_VERSION 6.02) #windows subsystem - arm minimum is 6.02
  elseif(CLR_CMAKE_HOST_ARCH_ARM64)
    set(WINDOWS_SUBSYSTEM_VERSION 6.03) #windows subsystem - arm64 minimum is 6.03
  endif ()

  #Do not create Side-by-Side Assembly Manifest
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/MANIFEST:NO>)
  # can handle addresses larger than 2 gigabytes
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/LARGEADDRESSAWARE>)
  #Compatible with Data Execution Prevention
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/NXCOMPAT>)
  #Use address space layout randomization
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/DYNAMICBASE>)
  #shrink pdb size
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/PDBCOMPRESS>)

  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/DEBUG>)
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/IGNORE:4197,4013,4254,4070,4221>)
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:/SUBSYSTEM:WINDOWS,${WINDOWS_SUBSYSTEM_VERSION}>)

  set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /IGNORE:4221")

  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:/DEBUG>)
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:/PDBCOMPRESS>)
  add_link_options($<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:/STACK:1572864>)

  # Debug build specific flags
  add_link_options($<$<AND:$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>>,$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>>:/NOVCFEATURE>)

  # Checked build specific flags
  add_link_options($<$<CONFIG:CHECKED>:/INCREMENTAL:NO>) # prevent "warning LNK4075: ignoring '/INCREMENTAL' due to '/OPT:REF' specification"
  add_link_options($<$<CONFIG:CHECKED>:/OPT:REF>)
  add_link_options($<$<CONFIG:CHECKED>:/OPT:NOICF>)

  # Release build specific flags
  add_link_options($<$<CONFIG:RELEASE>:/LTCG>)
  add_link_options($<$<CONFIG:RELEASE>:/OPT:REF>)
  add_link_options($<$<CONFIG:RELEASE>:/OPT:ICF>)
  set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")

  # ReleaseWithDebugInfo build specific flags
  add_link_options($<$<CONFIG:RELWITHDEBINFO>:/LTCG>)
  add_link_options($<$<CONFIG:RELWITHDEBINFO>:/OPT:REF>)
  add_link_options($<$<CONFIG:RELWITHDEBINFO>:/OPT:ICF>)
  set(CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_STATIC_LINKER_FLAGS_RELWITHDEBINFO} /LTCG")

  # Force uCRT to be dynamically linked for Release build
  add_link_options("$<$<CONFIG:RELEASE>:/NODEFAULTLIB:libucrt.lib;/DEFAULTLIB:ucrt.lib>")

elseif (CLR_CMAKE_HOST_UNIX)
  # Set the values to display when interactively configuring CMAKE_BUILD_TYPE
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "DEBUG;CHECKED;RELEASE;RELWITHDEBINFO")

  # Use uppercase CMAKE_BUILD_TYPE for the string comparisons below
  string(TOUPPER ${CMAKE_BUILD_TYPE} UPPERCASE_CMAKE_BUILD_TYPE)

  set(CLR_SANITIZE_CXX_OPTIONS "")
  set(CLR_SANITIZE_LINK_OPTIONS "")

  # set the CLANG sanitizer flags for debug build
  if(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL DEBUG OR UPPERCASE_CMAKE_BUILD_TYPE STREQUAL CHECKED)
    # obtain settings from running enablesanitizers.sh
    string(FIND "$ENV{DEBUG_SANITIZERS}" "asan" __ASAN_POS)
    string(FIND "$ENV{DEBUG_SANITIZERS}" "ubsan" __UBSAN_POS)
    if ((${__ASAN_POS} GREATER -1) OR (${__UBSAN_POS} GREATER -1))
      list(APPEND CLR_SANITIZE_CXX_OPTIONS -fsanitize-blacklist=${CMAKE_CURRENT_SOURCE_DIR}/sanitizerblacklist.txt)
      set (CLR_CXX_SANITIZERS "")
      set (CLR_LINK_SANITIZERS "")
      if (${__ASAN_POS} GREATER -1)
        list(APPEND CLR_CXX_SANITIZERS address)
        list(APPEND CLR_LINK_SANITIZERS address)
        set(CLR_SANITIZE_CXX_FLAGS "${CLR_SANITIZE_CXX_FLAGS}address,")
        set(CLR_SANITIZE_LINK_FLAGS "${CLR_SANITIZE_LINK_FLAGS}address,")
        add_definitions(-DHAS_ASAN)
        message("Address Sanitizer (asan) enabled")
      endif ()
      if (${__UBSAN_POS} GREATER -1)
        # all sanitizier flags are enabled except alignment (due to heavy use of __unaligned modifier)
        list(APPEND CLR_CXX_SANITIZERS
          "bool"
          bounds
          enum
          float-cast-overflow
          float-divide-by-zero
          "function"
          integer
          nonnull-attribute
          null
          object-size
          "return"
          returns-nonnull-attribute
          shift
          unreachable
          vla-bound
          vptr)
        list(APPEND CLR_LINK_SANITIZERS
          undefined)
        message("Undefined Behavior Sanitizer (ubsan) enabled")
      endif ()
      list(JOIN CLR_CXX_SANITIZERS "," CLR_CXX_SANITIZERS_OPTIONS)
      list(APPEND CLR_SANITIZE_CXX_OPTIONS "-fsanitize=${CLR_CXX_SANITIZERS_OPTIONS}")
      list(JOIN CLR_LINK_SANITIZERS "," CLR_LINK_SANITIZERS_OPTIONS)
      list(APPEND CLR_SANITIZE_LINK_OPTIONS "-fsanitize=${CLR_LINK_SANITIZERS_OPTIONS}")

      # -fdata-sections -ffunction-sections: each function has own section instead of one per .o file (needed for --gc-sections)
      # -O1: optimization level used instead of -O0 to avoid compile error "invalid operand for inline asm constraint"
      add_compile_definitions("$<$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>>:${CLR_SANITIZE_CXX_OPTIONS};-fdata-sections;--ffunction-sections;-O1>")
      add_link_options($<$<AND:$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>>,$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>>:${CLR_SANITIZE_LINK_OPTIONS}>)

      # -Wl and --gc-sections: drop unused sections\functions (similar to Windows /Gy function-level-linking)
      add_link_options("$<$<AND:$<OR:$<CONFIG:DEBUG>,$<CONFIG:CHECKED>>,$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>>:${CLR_SANITIZE_LINK_OPTIONS};-Wl,--gc-sections>")
    endif ()
  endif(UPPERCASE_CMAKE_BUILD_TYPE STREQUAL DEBUG OR UPPERCASE_CMAKE_BUILD_TYPE STREQUAL CHECKED)
endif(MSVC)

# CLR_ADDITIONAL_LINKER_FLAGS - used for passing additional arguments to linker
# CLR_ADDITIONAL_COMPILER_OPTIONS - used for passing additional arguments to compiler
#
# For example:
#       ./build-native.sh cmakeargs "-DCLR_ADDITIONAL_COMPILER_OPTIONS=<...>" cmakeargs "-DCLR_ADDITIONAL_LINKER_FLAGS=<...>"
#
if(CLR_CMAKE_HOST_UNIX)
    add_link_options(${CLR_ADDITIONAL_LINKER_FLAGS})
endif(CLR_CMAKE_HOST_UNIX)

if(CLR_CMAKE_HOST_LINUX)
  add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-Wa,--noexecstack>)
  add_link_options(-Wl,--build-id=sha1 -Wl,-z,relro,-z,now)
endif(CLR_CMAKE_HOST_LINUX)
if(CLR_CMAKE_HOST_FREEBSD)
  add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-Wa,--noexecstack>)
  add_link_options(-fuse-ld=lld LINKER:--build-id=sha1)
endif(CLR_CMAKE_HOST_FREEBSD)

#------------------------------------
# Definitions (for platform)
#-----------------------------------
if (CLR_CMAKE_HOST_ARCH_AMD64)
  add_definitions(-DHOST_AMD64)
  add_definitions(-DHOST_64BIT)
elseif (CLR_CMAKE_HOST_ARCH_I386)
  add_definitions(-DHOST_X86)
elseif (CLR_CMAKE_HOST_ARCH_ARM)
  add_definitions(-DHOST_ARM)
elseif (CLR_CMAKE_HOST_ARCH_ARM64)
  add_definitions(-DHOST_ARM64)
  add_definitions(-DHOST_64BIT)
else ()
  clr_unknown_arch()
endif ()

if (CLR_CMAKE_HOST_UNIX)
  if(CLR_CMAKE_HOST_LINUX)
    if(CLR_CMAKE_HOST_UNIX_AMD64)
      message("Detected Linux x86_64")
    elseif(CLR_CMAKE_HOST_UNIX_ARM)
      message("Detected Linux ARM")
    elseif(CLR_CMAKE_HOST_UNIX_ARM64)
      message("Detected Linux ARM64")
    elseif(CLR_CMAKE_HOST_UNIX_X86)
      message("Detected Linux i686")
    else()
      clr_unknown_arch()
    endif()
  endif(CLR_CMAKE_HOST_LINUX)
endif(CLR_CMAKE_HOST_UNIX)

if (CLR_CMAKE_HOST_UNIX)
  add_definitions(-DHOST_UNIX)

  if(CLR_CMAKE_HOST_DARWIN)
    message("Detected OSX x86_64")
  endif(CLR_CMAKE_HOST_DARWIN)

  if(CLR_CMAKE_HOST_FREEBSD)
    message("Detected FreeBSD amd64")
  endif(CLR_CMAKE_HOST_FREEBSD)

  if(CLR_CMAKE_HOST_NETBSD)
    message("Detected NetBSD amd64")
  endif(CLR_CMAKE_HOST_NETBSD)
endif(CLR_CMAKE_HOST_UNIX)

if (CLR_CMAKE_HOST_WIN32)
  add_definitions(-DHOST_WINDOWS)

  # Define the CRT lib references that link into Desktop imports
  set(STATIC_MT_CRT_LIB  "libcmt$<$<OR:$<CONFIG:Debug>,$<CONFIG:Checked>>:d>.lib")
  set(STATIC_MT_VCRT_LIB  "libvcruntime$<$<OR:$<CONFIG:Debug>,$<CONFIG:Checked>>:d>.lib")
  set(STATIC_MT_CPP_LIB  "libcpmt$<$<OR:$<CONFIG:Debug>,$<CONFIG:Checked>>:d>.lib")
endif(CLR_CMAKE_HOST_WIN32)

# Architecture specific files folder name
if (CLR_CMAKE_TARGET_ARCH_AMD64)
    set(ARCH_SOURCES_DIR amd64)
    add_definitions(-DTARGET_AMD64)
    add_definitions(-DTARGET_64BIT)
elseif (CLR_CMAKE_TARGET_ARCH_ARM64)
    set(ARCH_SOURCES_DIR arm64)
    add_definitions(-DTARGET_ARM64)
    add_definitions(-DTARGET_64BIT)
elseif (CLR_CMAKE_TARGET_ARCH_ARM)
    set(ARCH_SOURCES_DIR arm)
    add_definitions(-DTARGET_ARM)
elseif (CLR_CMAKE_TARGET_ARCH_I386)
    set(ARCH_SOURCES_DIR i386)
    add_definitions(-DTARGET_X86)
else ()
    clr_unknown_arch()
endif ()

#--------------------------------------
# Compile Options
#--------------------------------------
if (CLR_CMAKE_HOST_UNIX)
  # Disable frame pointer optimizations so profilers can get better call stacks
  add_compile_options(-fno-omit-frame-pointer)

  # The -fms-extensions enable the stuff like __if_exists, __declspec(uuid()), etc.
  add_compile_options(-fms-extensions)
  #-fms-compatibility      Enable full Microsoft Visual C++ compatibility
  #-fms-extensions         Accept some non-standard constructs supported by the Microsoft compiler

  # Make signed arithmetic overflow of addition, subtraction, and multiplication wrap around
  # using twos-complement representation (this is normally undefined according to the C++ spec).
  add_compile_options(-fwrapv)

  if(CLR_CMAKE_HOST_DARWIN)
    # We cannot enable "stack-protector-strong" on OS X due to a bug in clang compiler (current version 7.0.2)
    add_compile_options(-fstack-protector)
  else()
    check_cxx_compiler_flag(-fstack-protector-strong COMPILER_SUPPORTS_F_STACK_PROTECTOR_STRONG)
    if (COMPILER_SUPPORTS_F_STACK_PROTECTOR_STRONG)
      add_compile_options(-fstack-protector-strong)
    endif()
  endif(CLR_CMAKE_HOST_DARWIN)

  if (CLR_CMAKE_WARNINGS_ARE_ERRORS)
    # All warnings that are not explicitly disabled are reported as errors
    add_compile_options(-Werror)
  endif(CLR_CMAKE_WARNINGS_ARE_ERRORS)

  # Disabled common warnings
  add_compile_options(-Wno-unused-variable)
  add_compile_options(-Wno-unused-value)
  add_compile_options(-Wno-unused-function)

  #These seem to indicate real issues
  add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-invalid-offsetof>)

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # The -ferror-limit is helpful during the porting, it makes sure the compiler doesn't stop
    # after hitting just about 20 errors.
    add_compile_options(-ferror-limit=4096)

    # Disabled warnings
    add_compile_options(-Wno-unused-private-field)
    # Explicit constructor calls are not supported by clang (this->ClassName::ClassName())
    add_compile_options(-Wno-microsoft)
    # This warning is caused by comparing 'this' to NULL
    add_compile_options(-Wno-tautological-compare)
    # There are constants of type BOOL used in a condition. But BOOL is defined as int
    # and so the compiler thinks that there is a mistake.
    add_compile_options(-Wno-constant-logical-operand)
    # We use pshpack1/2/4/8.h and poppack.h headers to set and restore packing. However
    # clang 6.0 complains when the packing change lifetime is not contained within
    # a header file.
    add_compile_options(-Wno-pragma-pack)

    add_compile_options(-Wno-unknown-warning-option)

    # The following warning indicates that an attribute __attribute__((__ms_struct__)) was applied
    # to a struct or a class that has virtual members or a base class. In that case, clang
    # may not generate the same object layout as MSVC.
    add_compile_options(-Wno-incompatible-ms-struct)
  else()
    add_compile_options(-Wno-unused-but-set-variable)
    add_compile_options(-Wno-unknown-pragmas)
    add_compile_options(-Wno-uninitialized)
    add_compile_options(-Wno-strict-aliasing)
    add_compile_options(-Wno-array-bounds)
    check_cxx_compiler_flag(-Wclass-memaccess COMPILER_SUPPORTS_W_CLASS_MEMACCESS)
    if (COMPILER_SUPPORTS_W_CLASS_MEMACCESS)
      add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess>)
    endif()
    check_cxx_compiler_flag(-faligned-new COMPILER_SUPPORTS_F_ALIGNED_NEW)
    if (COMPILER_SUPPORTS_F_ALIGNED_NEW)
      add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-faligned-new>)
    endif()
  endif()

  # Some architectures (e.g., ARM) assume char type is unsigned while CoreCLR assumes char is signed
  # as x64 does. It has been causing issues in ARM (https://github.com/dotnet/coreclr/issues/4746)
  add_compile_options(-fsigned-char)

  # We mark the function which needs exporting with DLLEXPORT
  add_compile_options(-fvisibility=hidden)

  # Specify the minimum supported version of macOS
  if(CLR_CMAKE_HOST_DARWIN)
    set(MACOS_VERSION_MIN_FLAGS -mmacosx-version-min=10.12)
    add_compile_options(${MACOS_VERSION_MIN_FLAGS})
    add_link_options(${MACOS_VERSION_MIN_FLAGS})
  endif(CLR_CMAKE_HOST_DARWIN)
endif(CLR_CMAKE_HOST_UNIX)

if(CLR_CMAKE_TARGET_UNIX)
  add_definitions(-DTARGET_UNIX)
  # Contracts are disabled on UNIX.
  add_definitions(-DDISABLE_CONTRACTS)
  if(CLR_CMAKE_TARGET_DARWIN)
    add_definitions(-DTARGET_DARWIN)
  endif(CLR_CMAKE_TARGET_DARWIN)
  if(CLR_CMAKE_TARGET_FREEBSD)
    add_definitions(-DTARGET_FREEBSD)
  endif(CLR_CMAKE_TARGET_FREEBSD)
  if(CLR_CMAKE_TARGET_LINUX)
    add_definitions(-DTARGET_LINUX)
  endif(CLR_CMAKE_TARGET_LINUX)
  if(CLR_CMAKE_TARGET_NETBSD)
    add_definitions(-DTARGET_NETBSD)
  endif(CLR_CMAKE_TARGET_NETBSD)
else(CLR_CMAKE_TARGET_UNIX)
  add_definitions(-DTARGET_WINDOWS)
endif(CLR_CMAKE_TARGET_UNIX)

if(CLR_CMAKE_HOST_UNIX_ARM)
   if (NOT DEFINED CLR_ARM_FPU_TYPE)
     set(CLR_ARM_FPU_TYPE vfpv3)
   endif(NOT DEFINED CLR_ARM_FPU_TYPE)

   # Because we don't use CMAKE_C_COMPILER/CMAKE_CXX_COMPILER to use clang
   # we have to set the triple by adding a compiler argument
   add_compile_options(-mthumb)
   add_compile_options(-mfpu=${CLR_ARM_FPU_TYPE})
   if (NOT DEFINED CLR_ARM_FPU_CAPABILITY)
     set(CLR_ARM_FPU_CAPABILITY 0x7)
   endif(NOT DEFINED CLR_ARM_FPU_CAPABILITY)
   add_definitions(-DCLR_ARM_FPU_CAPABILITY=${CLR_ARM_FPU_CAPABILITY})
   add_compile_options(-march=armv7-a)
   if(ARM_SOFTFP)
     add_definitions(-DARM_SOFTFP)
     add_compile_options(-mfloat-abi=softfp)
   endif(ARM_SOFTFP)
endif(CLR_CMAKE_HOST_UNIX_ARM)

if(CLR_CMAKE_HOST_UNIX_X86)
  add_compile_options(-msse2)
endif()

if(CLR_CMAKE_HOST_UNIX)
  add_compile_options(${CLR_ADDITIONAL_COMPILER_OPTIONS})
endif(CLR_CMAKE_HOST_UNIX)

if (MSVC)
  # Compile options for targeting windows

  # The following options are set by the razzle build
  add_compile_options(/TP) # compile all files as C++
  add_compile_options(/d2Zi+) # make optimized builds debugging easier
  add_compile_options(/nologo) # Suppress Startup Banner
  add_compile_options(/W3) # set warning level to 3
  add_compile_options(/WX) # treat warnings as errors
  add_compile_options(/Oi) # enable intrinsics
  add_compile_options(/Oy-) # disable suppressing of the creation of frame pointers on the call stack for quicker function calls
  add_compile_options(/U_MT) # undefine the predefined _MT macro
  add_compile_options(/GF) # enable read-only string pooling
  add_compile_options(/Gm-) # disable minimal rebuild
  add_compile_options(/EHa) # enable C++ EH (w/ SEH exceptions)
  add_compile_options(/Zp8) # pack structs on 8-byte boundary
  add_compile_options(/Gy) # separate functions for linker
  add_compile_options(/Zc:wchar_t-) # C++ language conformance: wchar_t is NOT the native type, but a typedef
  add_compile_options(/Zc:forScope) # C++ language conformance: enforce Standard C++ for scoping rules
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-") # disable C++ RTTI
  add_compile_options(/FC) # use full pathnames in diagnostics
  add_compile_options(/MP) # Build with Multiple Processes (number of processes equal to the number of processors)
  add_compile_options(/GS) # Buffer Security Check
  add_compile_options(/Zm200) # Specify Precompiled Header Memory Allocation Limit of 150MB

  add_compile_options(/wd4960 /wd4961 /wd4603 /wd4627 /wd4838 /wd4456 /wd4457 /wd4458 /wd4459 /wd4091 /we4640)

  # Disable Warnings:
  # 4291: Delete not defined for new, c++ exception may cause leak.
  # 4302: Truncation from '%$S' to '%$S'.
  # 4311: Pointer truncation from '%$S' to '%$S'.
  # 4312: '<function-style-cast>' : conversion from '%$S' to '%$S' of greater size.
  # 4477: Format string '%$S' requires an argument of type '%$S', but variadic argument %d has type '%$S'.
  add_compile_options(/wd4291 /wd4302 /wd4311 /wd4312 /wd4477)

  # Treat Warnings as Errors:
  # 4007: 'main' : must be __cdecl.
  # 4013: 'function' undefined - assuming extern returning int.
  # 4102: "'%$S' : unreferenced label".
  # 4551: Function call missing argument list.
  # 4700: Local used w/o being initialized.
  # 4806: Unsafe operation involving type 'bool'.
  add_compile_options(/we4007 /we4013 /we4102 /we4551 /we4700 /we4806)

  # Set Warning Level 3:
  # 4092: Sizeof returns 'unsigned long'.
  # 4121: Structure is sensitive to alignment.
  # 4125: Decimal digit in octal sequence.
  # 4130: Logical operation on address of string constant.
  # 4132: Const object should be initialized.
  # 4212: Function declaration used ellipsis.
  # 4530: C++ exception handler used, but unwind semantics are not enabled. Specify -GX.
  add_compile_options(/w34092 /w34121 /w34125 /w34130 /w34132 /w34212 /w34530)

  # Set Warning Level 4:
  # 4177: Pragma data_seg s/b at global scope.
  add_compile_options(/w44177)

  add_compile_options(/Zi) # enable debugging information
  add_compile_options(/ZH:SHA_256) # use SHA256 for generating hashes of compiler processed source files.
  add_compile_options(/source-charset:utf-8) # Force MSVC to compile source as UTF-8.

  if (CLR_CMAKE_HOST_ARCH_I386)
    add_compile_options(/Gz)
  endif (CLR_CMAKE_HOST_ARCH_I386)

  add_compile_options($<$<OR:$<CONFIG:Release>,$<CONFIG:Relwithdebinfo>>:/GL>)
  add_compile_options($<$<OR:$<OR:$<CONFIG:Release>,$<CONFIG:Relwithdebinfo>>,$<CONFIG:Checked>>:/O1>)

  if (CLR_CMAKE_HOST_ARCH_AMD64)
  # The generator expression in the following command means that the /homeparams option is added only for debug builds
  add_compile_options($<$<CONFIG:Debug>:/homeparams>) # Force parameters passed in registers to be written to the stack
  endif (CLR_CMAKE_HOST_ARCH_AMD64)

  # enable control-flow-guard support for native components for non-Arm64 builds
  # Added using variables instead of add_compile_options to let individual projects override it
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /guard:cf")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /guard:cf")

  # Statically linked CRT (libcmt[d].lib, libvcruntime[d].lib and libucrt[d].lib) by default. This is done to avoid
  # linking in VCRUNTIME140.DLL for a simplified xcopy experience by reducing the dependency on VC REDIST.
  #
  # For Release builds, we shall dynamically link into uCRT [ucrtbase.dll] (which is pushed down as a Windows Update on downlevel OS) but
  # wont do the same for debug/checked builds since ucrtbased.dll is not redistributable and Debug/Checked builds are not
  # production-time scenarios.

  add_compile_options($<$<OR:$<OR:$<CONFIG:Release>,$<CONFIG:Relwithdebinfo>>,$<BOOL:$<TARGET_PROPERTY:DAC_COMPONENT>>>:/MT>)
  add_compile_options($<$<AND:$<OR:$<CONFIG:Debug>,$<CONFIG:Checked>>,$<NOT:$<BOOL:$<TARGET_PROPERTY:DAC_COMPONENT>>>>:/MTd>)

  add_compile_options($<$<COMPILE_LANGUAGE:ASM_MASM>:/ZH:SHA_256>)

  if (CLR_CMAKE_TARGET_ARCH_ARM OR CLR_CMAKE_TARGET_ARCH_ARM64)
    # Contracts work too slow on ARM/ARM64 DEBUG/CHECKED.
    add_definitions(-DDISABLE_CONTRACTS)
  endif (CLR_CMAKE_TARGET_ARCH_ARM OR CLR_CMAKE_TARGET_ARCH_ARM64)

endif (MSVC)

if(CLR_CMAKE_ENABLE_CODE_COVERAGE)

  if(CLR_CMAKE_HOST_UNIX)
    string(TOUPPER ${CMAKE_BUILD_TYPE} UPPERCASE_CMAKE_BUILD_TYPE)
    if(NOT UPPERCASE_CMAKE_BUILD_TYPE STREQUAL DEBUG)
      message( WARNING "Code coverage results with an optimised (non-Debug) build may be misleading" )
    endif(NOT UPPERCASE_CMAKE_BUILD_TYPE STREQUAL DEBUG)

    add_compile_options(-fprofile-arcs)
    add_compile_options(-ftest-coverage)
    add_link_options(--coverage)
  else()
    message(FATAL_ERROR "Code coverage builds not supported on current platform")
  endif(CLR_CMAKE_HOST_UNIX)

endif(CLR_CMAKE_ENABLE_CODE_COVERAGE)

if (CMAKE_BUILD_TOOL STREQUAL nmake)
  set(CMAKE_RC_CREATE_SHARED_LIBRARY "${CMAKE_CXX_CREATE_SHARED_LIBRARY}")
endif(CMAKE_BUILD_TOOL STREQUAL nmake)
