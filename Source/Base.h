#pragma once
#include <iostream>
#include <format>
#include <string>
#include <filesystem>

#define LOG(...) std::cout << std::format(__VA_ARGS__) << "\n";

#if defined(_MSC_VER)
    #define __DEBUGBREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    // std
    #include <signal.h>
    #define __DEBUGBREAK() raise(SIGTRAP)
#elif defined(__MINGW32__) || defined(__MINGW64__)
    #define __DEBUGBREAK() __builtin_trap()
#else
    #Error Unsupported compiler for debug break!
#endif

#define EXPAND_MACRO(x) x
#define STRINGIFY_MACRO(x) #x

#define __INTERNAL_ASSERT_IMPL(check, msg, ...) { if(!(check)) { LOG(msg, __VA_ARGS__); __DEBUGBREAK(); } }
#define __INTERNAL_ASSERT_WITH_MSG(check, ...) __INTERNAL_ASSERT_IMPL(check, "Assertion failed: {}", __VA_ARGS__)
#define __INTERNAL_ASSERT_NO_MSG(check) __INTERNAL_ASSERT_IMPL(check, "Assertion '{0}' failed at {1}:{2}", STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define __INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define __INTERNAL_ASSERT_GET_MACRO(...) EXPAND_MACRO( __INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, __INTERNAL_ASSERT_WITH_MSG, __INTERNAL_ASSERT_NO_MSG) )

#define ASSERT(...) EXPAND_MACRO( __INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(__VA_ARGS__))