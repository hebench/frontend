// Copyright (C) 2021 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#ifndef __COMMON_Interface_H_7e5fa8c2415240ea93eff148ed73539b
#define __COMMON_Interface_H_7e5fa8c2415240ea93eff148ed73539b

// This header contains the proper declarations required for exported
// symbols from a shared library in Windows (dll) and Linux (so).
//
// To use, simply append the propper COMMON_INTERFACE_XXX macro to the
// symbol that needs to be exported (function, structure, class, etc.)
// and define macro IMPLEMENT_COMMON_INTERFACE_API at compiler level
// when building the shared library (macro must not be defined when
// building external code that will be importing the library).

#if defined(_WIN32)
#define COMMON_INTERFACE_CDECL

#ifdef IMPLEMENT_COMMON_INTERFACE_API // to be defined when building as DLL
#define COMMON_INTERFACE_API(...)       extern "C" __declspec(dllexport) __VA_ARGS__ __cdecl
#define COMMON_INTERFACE_API_CPP(...)   __declspec(dllexport) __VA_ARGS__ __cdecl
#define COMMON_INTERFACE_API_CLASS(...) __declspec(dllexport) __VA_ARGS__
#else
#define COMMON_INTERFACE_API(...)       extern "C" __declspec(dllimport) __VA_ARGS__ __cdecl
#define COMMON_INTERFACE_API_CPP(...)   __declspec(dllimport) __VA_ARGS__ __cdecl
#define COMMON_INTERFACE_API_CLASS(...) __declspec(dllimport) __VA_ARGS__
#endif

#else
#define COMMON_INTERFACE_CDECL          __attribute__((cdecl))
#define COMMON_INTERFACE_API(...)       extern "C" __attribute__((visibility("default"))) __VA_ARGS__
#define COMMON_INTERFACE_API_CPP(...)   __attribute__((visibility("default"))) __VA_ARGS__
#define COMMON_INTERFACE_API_CLASS(...) __attribute__((visibility("default"))) __VA_ARGS__
#endif

#if __cplusplus >= 201402L
#define COMMON_INTERFACE_DEPRECATED(msg) [[deprecated(msg)]]
#elif defined(_MSC_VER)
#define COMMON_INTERFACE_DEPRECATED(msg) __declspec(deprecated(msg))
#elif defined __INTEL_COMPILER
#define COMMON_INTERFACE_DEPRECATED(msg) __attribute__((deprecated(msg)))
#elif defined(__GNUC__)
#define COMMON_INTERFACE_DEPRECATED(msg) __attribute__((deprecated((msg))))
#else
#define COMMON_INTERFACE_DEPRECATED(msg)
#endif

#endif // defined __COMMON_Interface_H_7e5fa8c2415240ea93eff148ed73539b
