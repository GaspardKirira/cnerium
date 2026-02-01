#pragma once

// ================================
// Platform detection
// ================================

#if defined(_WIN32)
#  define CNERIUM_PLATFORM_WINDOWS 1
#else
#  define CNERIUM_PLATFORM_WINDOWS 0
#endif

#if defined(__linux__)
#  define CNERIUM_PLATFORM_LINUX 1
#else
#  define CNERIUM_PLATFORM_LINUX 0
#endif

#if defined(__APPLE__)
#  define CNERIUM_PLATFORM_APPLE 1
#else
#  define CNERIUM_PLATFORM_APPLE 0
#endif

#if defined(__unix__) || CNERIUM_PLATFORM_LINUX || CNERIUM_PLATFORM_APPLE
#  define CNERIUM_PLATFORM_UNIX 1
#else
#  define CNERIUM_PLATFORM_UNIX 0
#endif

// ================================
// Architecture detection
// ================================

#if defined(__x86_64__) || defined(_M_X64)
#  define CNERIUM_ARCH_X64 1
#else
#  define CNERIUM_ARCH_X64 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#  define CNERIUM_ARCH_ARM64 1
#else
#  define CNERIUM_ARCH_ARM64 0
#endif
