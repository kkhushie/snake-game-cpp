// lab4/gtest_lite.h
// Minimal GTest-compatible macro subset for MinGW 6.3.0 / C++11 compatibility.
// Implements: TEST(), EXPECT_TRUE, EXPECT_FALSE, EXPECT_EQ, RUN_ALL_TESTS.
// The macro API mirrors GTest so tests can compile with real GTest when available.

#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include <cstdlib>

namespace gtest_lite {

struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> fn;
};

// C++11-safe: use a function-local static (no inline variables)
inline std::vector<TestCase>& registry() {
    static std::vector<TestCase> v;
    return v;
}

// Runner state as a singleton struct (avoids inline variables, C++11 safe)
struct State {
    int  passed = 0;
    int  failed = 0;
    bool cur_failed = false;
    std::string cur_test;
};

inline State& state() {
    static State s;
    return s;
}

struct Registrar {
    Registrar(const char* suite, const char* name, std::function<void()> fn) {
        registry().push_back({suite, name, fn});
    }
};

inline void fail_msg(const char* file, int line, const std::string& msg) {
    std::cerr << file << ":" << line << ": FAILED\n    " << msg << "\n";
    state().cur_failed = true;
}

template<typename A, typename B>
inline bool eq_helper(const A& a, const B& b) { return a == b; }

inline int run_all_tests() {
    for (auto& tc : registry()) {
        state().cur_failed = false;
        state().cur_test = tc.suite + "." + tc.name;
        tc.fn();
        if (state().cur_failed) {
            std::cerr << "[FAILED] " << state().cur_test << "\n";
            ++state().failed;
        } else {
            std::cout << "[  OK  ] " << state().cur_test << "\n";
            ++state().passed;
        }
    }
    int total = state().passed + state().failed;
    std::cout << "\n" << state().passed << "/" << total << " tests passed.\n";
    return state().failed > 0 ? 1 : 0;
}

} // namespace gtest_lite

// --- Public macros (GTest-compatible API) ---

#define TEST(Suite, Name) \
    static void _test_##Suite##_##Name(); \
    static ::gtest_lite::Registrar _reg_##Suite##_##Name(#Suite, #Name, _test_##Suite##_##Name); \
    static void _test_##Suite##_##Name()

// Dummy type that absorbs << message chains (for GTest compatibility)
struct _GtLiteMsgSink {
    template<typename T> _GtLiteMsgSink operator<<(const T&) const { return {}; }
};

#define EXPECT_TRUE(expr) \
    ([&]() -> ::_GtLiteMsgSink { \
        if (!(expr)) ::gtest_lite::fail_msg(__FILE__, __LINE__, "Expected true: " #expr); \
        return {}; \
    }())

#define EXPECT_FALSE(expr) \
    ([&]() -> ::_GtLiteMsgSink { \
        if ((expr)) ::gtest_lite::fail_msg(__FILE__, __LINE__, "Expected false: " #expr); \
        return {}; \
    }())

#define EXPECT_EQ(a, b) \
    ([&]() -> ::_GtLiteMsgSink { \
        auto _a = (a); auto _b = (b); \
        if (!::gtest_lite::eq_helper(_a, _b)) { \
            std::ostringstream _oss; \
            _oss << "Expected " #a " == " #b " but got " << _a << " != " << _b; \
            ::gtest_lite::fail_msg(__FILE__, __LINE__, _oss.str()); \
        } \
        return {}; \
    }())

#define RUN_ALL_TESTS() ::gtest_lite::run_all_tests()

#define GTEST_MAIN() \
    int main(int, char**) { return RUN_ALL_TESTS(); }
