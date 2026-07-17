#include "test_harness.hpp"

#include <probot/io/battery.hpp>

using namespace probot::io::battery::detail;

TEST_CASE(battery_ema_starts_at_first_sample){
  // 0'dan tırmanmak yerine ilk örneğe atlar — açılışta dakikalarca yanlış
  // "düşük pil" göstermesin.
  float v = emaStep(0.0f, 12.4f, 0.2f);
  EXPECT_NEAR(v, 12.4f, 1e-6f);
}

TEST_CASE(battery_ema_smooths_toward_sample){
  float v = 12.0f;
  v = emaStep(v, 13.0f, 0.2f);
  EXPECT_NEAR(v, 12.2f, 1e-5f);
  // Tek büyük sıçrama tamamen geçmez (yumuşatma amacı).
  EXPECT_TRUE(v < 12.5f);
}

TEST_CASE(battery_ema_converges){
  float v = 10.8f;
  for (int i = 0; i < 60; i++) v = emaStep(v, 12.6f, 0.2f);
  EXPECT_NEAR(v, 12.6f, 0.01f);
}

TEST_CASE(battery_decivolts_rounding_and_bounds){
  EXPECT_TRUE(toDecivolts(0.0f) == 0u);
  EXPECT_TRUE(toDecivolts(-1.0f) == 0u);
  EXPECT_TRUE(toDecivolts(12.44f) == 124u);
  EXPECT_TRUE(toDecivolts(12.45f) == 125u);  // yarım yukarı
  EXPECT_TRUE(toDecivolts(500.0f) == 999u);  // tavan 99.9 V
}

TEST_CASE(battery_decivolts_nan_is_no_data){
  EXPECT_TRUE(toDecivolts(__builtin_nanf("")) == 0u);
}

TEST_CASE(battery_divider_ratio){
  // 100k / 22k: 12.6 V -> 2.27 V (S3 12dB doğrusal bölge içinde)
  float r = dividerRatio(100.0f, 22.0f);
  EXPECT_NEAR(r, 122.0f / 22.0f, 1e-5f);
  EXPECT_NEAR(12.6f / r, 2.272f, 0.01f);
  // Alt direnç 0/negatif: geçersiz konfig, 0 döner (bölme hatası yok)
  EXPECT_TRUE(dividerRatio(100.0f, 0.0f) == 0.0f);
}
