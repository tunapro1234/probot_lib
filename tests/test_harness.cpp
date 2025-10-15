#include "test_harness.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

unsigned long _test_millis_now = 0;

namespace {
  using Entry = std::pair<std::string, test::TestFn>;
  std::vector<Entry>& registry(){
    static std::vector<Entry> tests;
    return tests;
  }
  std::size_t& failure_counter(){
    static std::size_t count = 0;
    return count;
  }
}

namespace probot::builtinled {
  static bool g_last_on = false;
  static uint8_t g_last_r = 0;
  static uint8_t g_last_g = 0;
  static uint8_t g_last_b = 0;
  static uint8_t g_last_brightness = 0;

  void set(bool on){ g_last_on = on; }
  void setBrightness(uint8_t brightness){ g_last_brightness = brightness; }
  void setColor(uint8_t r, uint8_t g, uint8_t b){ g_last_r = r; g_last_g = g; g_last_b = b; }

  bool test_last_on(){ return g_last_on; }
  uint8_t test_last_brightness(){ return g_last_brightness; }
  uint8_t test_last_r(){ return g_last_r; }
  uint8_t test_last_g(){ return g_last_g; }
  uint8_t test_last_b(){ return g_last_b; }
  void test_reset(){ g_last_on=false; g_last_r=g_last_g=g_last_b=g_last_brightness=0; }
}

namespace test {
  void register_test(const char* name, TestFn fn){ registry().emplace_back(name, fn); }
  void reset_failures(){ failure_counter() = 0; }
  std::size_t failure_count(){ return failure_counter(); }

  int run_all_tests(){
    std::cout << "Running " << registry().size() << " tests...\n";
    std::size_t idx = 0;
    for (const auto& [name, fn] : registry()){
      try {
        fn();
        std::cout << "[PASS] " << name << "\n";
      } catch (const std::exception& ex){
        ++failure_counter();
        std::cout << "[FAIL] " << name << ": " << ex.what() << "\n";
      } catch (...){
        ++failure_counter();
        std::cout << "[FAIL] " << name << ": unknown error\n";
      }
      ++idx;
    }
    if (failure_counter() == 0){
      std::cout << "All tests passed\n";
      return 0;
    }
    std::cout << failure_counter() << " test(s) failed\n";
    return 1;
  }
}

int main(){
  return test::run_all_tests();
}
