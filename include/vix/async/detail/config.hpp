/**
 *
 *  @file config.hpp
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
#ifndef VIX_ASYNC_CONFIG_HPP
#define VIX_ASYNC_CONFIG_HPP

/**
 * @brief Versioning macros for the async module.
 *
 * These macros define the semantic version of the async runtime.
 * They can be overridden externally before including this header
 * if needed by build systems or package managers.
 */

/** @brief Major version number. */
#ifndef ASYNC_VERSION_MAJOR
#define ASYNC_VERSION_MAJOR 0
#endif

/** @brief Minor version number. */
#ifndef ASYNC_VERSION_MINOR
#define ASYNC_VERSION_MINOR 1
#endif

/** @brief Patch version number. */
#ifndef ASYNC_VERSION_PATCH
#define ASYNC_VERSION_PATCH 0
#endif

/**
 * @brief Human-readable version string.
 *
 * Must remain consistent with ASYNC_VERSION_MAJOR / MINOR / PATCH.
 */
#define ASYNC_VERSION_STRING "0.1.0"

/**
 * @brief Enable or disable runtime assertions.
 *
 * If ASYNC_ENABLE_ASSERTS is not explicitly defined:
 * - assertions are enabled in debug builds (!NDEBUG)
 * - assertions are disabled in release builds (NDEBUG)
 */
#ifndef ASYNC_ENABLE_ASSERTS
#ifndef NDEBUG
#define ASYNC_ENABLE_ASSERTS 1
#else
#define ASYNC_ENABLE_ASSERTS 0
#endif
#endif

/**
 * @brief Symbol visibility macros.
 *
 * These macros control symbol export/import for shared library builds.
 *
 * - On Windows:
 *   - ASYNC_EXPORT expands to __declspec(dllexport)
 *   - ASYNC_IMPORT expands to __declspec(dllimport)
 *
 * - On non-Windows platforms:
 *   - ASYNC_EXPORT expands to default visibility attribute
 *   - ASYNC_IMPORT expands to nothing
 */
#if defined(_WIN32)
/** @brief Export symbol from a DLL. */
#define ASYNC_EXPORT __declspec(dllexport)
/** @brief Import symbol from a DLL. */
#define ASYNC_IMPORT __declspec(dllimport)
#else
/** @brief Export symbol with default visibility. */
#define ASYNC_EXPORT __attribute__((visibility("default")))
/** @brief Import symbol (no-op on non-Windows platforms). */
#define ASYNC_IMPORT
#endif

#endif // VIX_ASYNC_CONFIG_HPP
