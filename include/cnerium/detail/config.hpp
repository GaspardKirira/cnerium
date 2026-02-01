/**
 *
 *  @file config.hpp
 *  @author Gaspard Kirira
 *
 *  Copyright 2025, Gaspard Kirira.  All rights reserved.
 *  https://github.com/GaspardKirira/cnerium
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  CNERIUM
 *
 */
#ifndef CNERIUM_CONFIG_HPP
#define CNERIUM_CONFIG_HPP

#ifndef CNERIUM_VERSION_MAJOR
#define CNERIUM_VERSION_MAJOR 0
#endif

#ifndef CNERIUM_VERSION_MINOR
#define CNERIUM_VERSION_MINOR 1
#endif

#ifndef CNERIUM_VERSION_PATCH
#define CNERIUM_VERSION_PATCH 0
#endif

#define CNERIUM_VERSION_STRING "0.1.0"

#ifndef CNERIUM_ENABLE_ASSERTS
#ifndef NDEBUG
#define CNERIUM_ENABLE_ASSERTS 1
#else
#define CNERIUM_ENABLE_ASSERTS 0
#endif
#endif

#if defined(_WIN32)
#define CNERIUM_EXPORT __declspec(dllexport)
#define CNERIUM_IMPORT __declspec(dllimport)
#else
#define CNERIUM_EXPORT __attribute__((visibility("default")))
#define CNERIUM_IMPORT
#endif

#endif
