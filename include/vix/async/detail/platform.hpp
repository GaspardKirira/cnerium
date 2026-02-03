/**
 *
 *  @file platform.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.
 *  All rights reserved.
 *  https://github.com/vixcpp/vix
 *
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Vix.cpp
 *
 */
#ifndef VIX_ASYNC_PLATFORM_HPP
#define VIX_ASYNC_PLATFORM_HPP

/**
 * @brief Platform and architecture detection macros for the async module.
 *
 * This header provides a small set of preprocessor macros that describe:
 * - the target operating system (Windows, Linux, Apple, Unix)
 * - the target CPU architecture (x86_64, ARM64)
 *
 * All macros expand to either 1 (true) or 0 (false).
 */

// Platform detection
#if defined(_WIN32)
#define ASYNC_PLATFORM_WINDOWS 1 /**< Defined as 1 when targeting Windows. */
#else
#define ASYNC_PLATFORM_WINDOWS 0 /**< Defined as 0 when not targeting Windows. */
#endif

#if defined(__linux__)
#define ASYNC_PLATFORM_LINUX 1 /**< Defined as 1 when targeting Linux. */
#else
#define ASYNC_PLATFORM_LINUX 0 /**< Defined as 0 when not targeting Linux. */
#endif

#if defined(__APPLE__)
#define ASYNC_PLATFORM_APPLE 1 /**< Defined as 1 when targeting Apple platforms (macOS, iOS). */
#else
#define ASYNC_PLATFORM_APPLE 0 /**< Defined as 0 when not targeting Apple platforms. */
#endif

#if defined(__unix__) || ASYNC_PLATFORM_LINUX || ASYNC_PLATFORM_APPLE
#define ASYNC_PLATFORM_UNIX 1 /**< Defined as 1 when targeting a Unix-like platform. */
#else
#define ASYNC_PLATFORM_UNIX 0 /**< Defined as 0 when not targeting a Unix-like platform. */
#endif

// Architecture detection
#if defined(__x86_64__) || defined(_M_X64)
#define ASYNC_ARCH_X64 1 /**< Defined as 1 when targeting x86_64. */
#else
#define ASYNC_ARCH_X64 0 /**< Defined as 0 when not targeting x86_64. */
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define ASYNC_ARCH_ARM64 1 /**< Defined as 1 when targeting ARM64 (AArch64). */
#else
#define ASYNC_ARCH_ARM64 0 /**< Defined as 0 when not targeting ARM64. */
#endif

#endif // VIX_ASYNC_PLATFORM_HPP
