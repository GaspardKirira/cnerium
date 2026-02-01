# cmake/CneriumOptions.cmake
# Centralized project options for Cnerium.

include_guard(GLOBAL)

option(CNERIUM_BUILD_TESTS "Build Cnerium tests" ON)
option(CNERIUM_BUILD_EXAMPLES "Build Cnerium examples" OFF)

option(CNERIUM_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

option(CNERIUM_ENABLE_SANITIZERS "Enable AddressSanitizer + UBSan (where supported)" OFF)
option(CNERIUM_ENABLE_TSAN "Enable ThreadSanitizer (where supported)" OFF)

option(CNERIUM_USE_MOLD "Use mold linker when available (Linux only)" OFF)

# Default build type for single-config generators (Ninja/Makefiles)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# Optional: mold linker
if(CNERIUM_USE_MOLD AND UNIX AND NOT APPLE)
  # mold usage is best-effort
  find_program(CNERIUM_MOLD_EXE mold)
  if(CNERIUM_MOLD_EXE)
    message(STATUS "Cnerium: using mold linker: ${CNERIUM_MOLD_EXE}")
    add_link_options(-fuse-ld=mold)
  else()
    message(STATUS "Cnerium: mold requested but not found")
  endif()
endif()
