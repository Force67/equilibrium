#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include "crc.h"

using namespace base;

TEST(CRC32CTest, HardwareCRC32) {
  const char data1[] = "12345";
  uint32_t crc1 =
      CRC32C(reinterpret_cast<const uint8_t*>(data1),
               sizeof(data1) - 1);  // Subtract 1 to avoid null terminator
  EXPECT_EQ(crc1, 0xa25caaff);

  const char data2[] = "45";
  uint32_t crc2 =
      CRC32C(reinterpret_cast<const uint8_t*>(data2),
                           sizeof(data2) - 1, ~crc1);
  EXPECT_EQ(crc2, 0x9e63ca4c);
}

TEST(CRC32CTest, SoftwareCRC32) {
  const char data1[] = "123";
  uint32_t crc1 =
      CRC32C_SW(reinterpret_cast<const uint8_t*>(data1), sizeof(data1) - 1);
  EXPECT_EQ(crc1, 0x8fe07337);
}

TEST(CRC32CTest, HardwareAndSoftwareConsistency) {
  const char data[] = "12345";
  uint32_t crc_hw =
      CRC32C(reinterpret_cast<const uint8_t*>(data), sizeof(data) - 1 /*skip nterm*/);
  uint32_t crc_sw =
      CRC32C_SW(reinterpret_cast<const uint8_t*>(data), sizeof(data) - 1);
  EXPECT_EQ(crc_hw, crc_sw);
}

// see https://crccalc.com/
TEST(CRC32Test, SoftwareCRC32) {
  const char data1[] = "123456789";
  uint32_t crc1 =
      CRC32_SW(reinterpret_cast<const uint8_t*>(data1), sizeof(data1) - 1);
  EXPECT_EQ(crc1, 0xCBF43926);
}