cmake_minimum_required(VERSION 3.20)

# ----------------------------------------
# Project
# ----------------------------------------
project(cnerium
  VERSION 0.1.0
  DESCRIPTION "Cnerium: async C++ core (scheduler, tasks, timers, cancellation)"
  LANGUAGES CXX
)

# ----------------------------------------
# Options + helpers
# ----------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CneriumOptions)
include(CneriumWarnings)
include(CneriumSanitizers)

# ----------------------------------------
# Library target
# ----------------------------------------
add_library(cnerium)

add_library(cnerium::cnerium ALIAS cnerium)

target_compile_features(cnerium PUBLIC cxx_std_20)

target_include_directories(cnerium
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# If you later want to split headers-only parts, keep this list explicit.
target_sources(cnerium
  PRIVATE
    src/core/scheduler.cpp
    src/core/thread_pool.cpp
    src/core/timer.cpp
    src/core/signal.cpp
    src/net/asio_net_service.cpp
    src/net/asio_tcp.cpp
    src/net/asio_udp.cpp
    src/net/asio_dns.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      include/cnerium/version.hpp
      include/cnerium/core/io_context.hpp
      include/cnerium/core/scheduler.hpp
      include/cnerium/core/task.hpp
      include/cnerium/core/cancel.hpp
      include/cnerium/core/timer.hpp
      include/cnerium/core/signal.hpp
      include/cnerium/core/thread_pool.hpp
      include/cnerium/core/error.hpp
      include/cnerium/core/spawn.hpp
)

# ----------------------------------------
# Warnings / sanitizers
# ----------------------------------------
cnerium_apply_warnings(cnerium)
cnerium_apply_sanitizers(cnerium)

# ----------------------------------------
# Dependencies (placeholder)
# ----------------------------------------
# We keep it empty for now. Later, if you choose Asio standalone:
# find_package(asio CONFIG REQUIRED)  # or FetchContent
# target_link_libraries(cnerium PUBLIC asio::asio)

# ----------------------------------------
# Build settings
# ----------------------------------------
set_target_properties(cnerium PROPERTIES
  CXX_EXTENSIONS OFF
  POSITION_INDEPENDENT_CODE ON
)

include(FetchContent)

FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-30-2
)
FetchContent_MakeAvailable(asio)

target_compile_definitions(cnerium PUBLIC ASIO_STANDALONE)
if(UNIX AND NOT APPLE)
  target_link_libraries(cnerium PUBLIC pthread)
endif()


# ----------------------------------------
# Tests / Examples
# ----------------------------------------
include(CTest)

if(CNERIUM_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(CNERIUM_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# ----------------------------------------
# Install + export (for find_package)
# ----------------------------------------
include(GNUInstallDirs)

install(
  TARGETS cnerium
  EXPORT cneriumTargets
  FILE_SET HEADERS
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
  EXPORT cneriumTargets
  NAMESPACE cnerium::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/cnerium
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/cneriumConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/cneriumConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cneriumConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cnerium"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/cneriumConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/cneriumConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/cnerium"
)
