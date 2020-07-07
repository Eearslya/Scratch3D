#pragma once

#define FORCEINLINE __forceinline
#define FORCENOINLINE _declspec(noinline)
#define ALIGN(n) __declspec(align(n))
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#ifndef NO_ASSERT
#define ASSERT_ENABLE
#endif

#ifdef ASSERT_ENABLE
#include <iostream>
#if _MSC_VER
#include <intrin.h>
#define DebugBreak() __debugbreak();
#else
#define DebugBreak() __asm {int 3}
#endif

#define ASSERT_MSG(expr, msg)                               \
  {                                                         \
    if (expr) {                                             \
    } else {                                                \
      AssertionFailure(#expr, msg, __FILE__, __LINE__); \
      DebugBreak();                                     \
    }                                                       \
  }

#define ASSERT(expr) ASSERT_MSG(expr, "")

#ifdef _DEBUG
#define ASSERT_DEBUG(expr) ASSERT(expr)
#define ASSERT_DEBUG_MSG(expr, msg) ASSERT_MSG(expr, msg)
#else
#define ASSERT_DEBUG(expr)
#define ASSERT_DEBUG_MSG(expr, msg)
#endif

FORCEINLINE void AssertionFailure(const char* expression, const char* msg,
                                      const char* file, int line) {
  std::cerr << "Assertion Failure: " << expression << "\n";
  std::cerr << "  Message: " << msg << "\n";
  std::cerr << "  At: " << file << ":" << line << "\n";
}

#else
#define ASSERT(expr)
#define ASSERT_MSG(expr, msg)
#define ASSERT_DEBUG(expr)
#define ASSERT_DEBUG_MSG(expr, msg)
#endif