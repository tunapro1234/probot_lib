#pragma once
#include <cstddef>
#include <cmath>
#include <stdexcept>

namespace test {
  using TestFn = void(*)();

  void register_test(const char* name, TestFn fn);
  int run_all_tests();
  void reset_failures();
  std::size_t failure_count();
}

#define TEST_CASE(NAME) \
  static void NAME(); \
  namespace { \
    struct NAME##_registrar { \
      NAME##_registrar(){ test::register_test(#NAME, &NAME); } \
    }; \
    static NAME##_registrar NAME##_registrar_instance; \
  } \
  static void NAME()

#define EXPECT_TRUE(cond) do { if (!(cond)) throw std::runtime_error("EXPECT_TRUE failed: " #cond); } while(0)
#define EXPECT_NEAR(a,b,eps) do { if (std::fabs((a) - (b)) > (eps)) throw std::runtime_error("EXPECT_NEAR failed: " #a " vs " #b); } while(0)
