#include <gtest/gtest.h>
#include <base/memory/unique_pointer.h>
#include <base/numeric_limits.h>
#include "memory_mapped_file.h"
#include "scoped_temp_dir.h"

namespace base {

// A ReMap offset has to be aligned: mmap wants a page multiple, and
// MapViewOfFile wants a multiple of the Windows allocation granularity, which
// is 64 KiB. 64 KiB satisfies both. Three of them, so a ReMap can cover the
// middle slice and leave file bytes outside the view on either side.
constexpr mem_size kGranularity = 64 * 1024;
constexpr mem_size kPayloadSize = kGranularity * 3;

class MemoryMappedFileTest : public ::testing::Test {
 protected:
  base::UniquePointer<MemoryMappedFile> mmf;
  base::File temp_file;
  // base::ScopedTempDir temp_dir;

  // payload[i] is derived from i, so any byte read back identifies the file
  // offset it actually came from.
  static byte ExpectedByteAt(mem_size offset) {
    return static_cast<byte>((offset * 31u + 7u) & 0xFF);
  }

  // Fails the current test at the first byte that did not come from
  // |file_offset|. A per-byte ASSERT_EQ over 64 KiB is needlessly slow.
  static void ExpectMatchesFile(u64 file_offset, const base::Vector<byte>& data) {
    for (mem_size i = 0; i < data.size(); ++i) {
      if (data[i] != ExpectedByteAt(file_offset + i)) {
        FAIL() << "byte from the wrong place at file offset " << (file_offset + i);
      }
    }
  }

  void SetUp() override {
    // ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    // auto path = temp_dir.path();
    // path.Append("temp_file");
    base::Path path("./temp_file");
    temp_file.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE |
                                   base::File::FLAG_READ);
    ASSERT_TRUE(temp_file.IsValid());

    // On the heap: kPayloadSize is 192 KiB, which is more than a thread stack
    // should be asked to hold.
    base::Vector<char> payload(kPayloadSize);
    for (mem_size i = 0; i < kPayloadSize; ++i)
      payload[i] = static_cast<char>(ExpectedByteAt(i));
    ASSERT_EQ(temp_file.Write(0, payload.data(), kPayloadSize),
              static_cast<int>(kPayloadSize));
    ASSERT_TRUE(temp_file.Flush());

    mmf = base::MakeUnique<MemoryMappedFile>(temp_file);
  }

  void TearDown() override {
    // MemoryMappedFile has no destructor, so the view and the descriptor only
    // go back if Close() is called.
    if (mmf)
      mmf->Close();
    mmf.Reset();
    temp_file.Close();
  }
};

TEST_F(MemoryMappedFileTest, MapSuccess) {
  EXPECT_TRUE(mmf->Map());
}

TEST_F(MemoryMappedFileTest, MapCoversTheWholeFile) {
  ASSERT_TRUE(mmf->Map());
  EXPECT_EQ(mmf->file_size(), kPayloadSize);
  EXPECT_EQ(mmf->view_offset(), 0u);
  EXPECT_EQ(mmf->view_size(), kPayloadSize);

  const base::Vector<byte> data = mmf->Read(0, kPayloadSize);
  ASSERT_EQ(data.size(), kPayloadSize);
  ExpectMatchesFile(0, data);
}

TEST_F(MemoryMappedFileTest, ReadRejectsAnOffsetThatWrapsTheRangeCheck) {
  ASSERT_TRUE(mmf->Map());
  // offset + size wraps to 4, which is inside the file. The old check was
  // `offset + size > file_size_`, so this range passed it and the memcpy read
  // from the view plus a near-maximum offset.
  const u64 offset = base::MinMax<u64>::max() - 3;
  EXPECT_TRUE(mmf->Read(offset, 8).empty());
}

TEST_F(MemoryMappedFileTest, WriteRejectsAnOffsetThatWrapsTheRangeCheck) {
  ASSERT_TRUE(mmf->Map());
  byte payload[8]{};
  const u64 offset = base::MinMax<u64>::max() - 3;
  EXPECT_FALSE(mmf->Write(offset, base::Span<byte>(payload, 8)));
}

TEST_F(MemoryMappedFileTest, ReadRejectsARangeThatEndsPastTheView) {
  ASSERT_TRUE(mmf->Map());
  EXPECT_TRUE(mmf->Read(kPayloadSize - 4, 8).empty());
  EXPECT_TRUE(mmf->Read(kPayloadSize, 1).empty());
  // The last byte in range is still readable.
  EXPECT_EQ(mmf->Read(kPayloadSize - 1, 1).size(), 1u);
}

TEST_F(MemoryMappedFileTest, ReadRejectsEverythingBeforeMap) {
  EXPECT_TRUE(mmf->Read(0, 1).empty());
  byte payload[1]{};
  EXPECT_FALSE(mmf->Write(0, base::Span<byte>(payload, 1)));
}

TEST_F(MemoryMappedFileTest, ReMapNarrowsTheReadableRange) {
  ASSERT_TRUE(mmf->Map());
  ASSERT_TRUE(mmf->ReMap(kGranularity, kGranularity));
  EXPECT_EQ(mmf->view_offset(), kGranularity);
  EXPECT_EQ(mmf->view_size(), kGranularity);

  // Read takes file offsets, and the view -- not the file size -- bounds them.
  // Offset 0 is inside the file but outside the view, and it used to be
  // accepted and then indexed off the start of the view.
  EXPECT_TRUE(mmf->Read(0, 1).empty());
  EXPECT_TRUE(mmf->Read(kGranularity * 2, 1).empty());

  const base::Vector<byte> data = mmf->Read(kGranularity, kGranularity);
  ASSERT_EQ(data.size(), kGranularity);
  // The bytes must be the ones at that file offset, which only holds if the
  // view offset is subtracted out before indexing.
  ExpectMatchesFile(kGranularity, data);
}

TEST_F(MemoryMappedFileTest, ReMapClampsALengthPastTheEndOfTheFile) {
  ASSERT_TRUE(mmf->Map());
  ASSERT_TRUE(mmf->ReMap(kGranularity * 2, kGranularity * 8));
  EXPECT_EQ(mmf->view_offset(), kGranularity * 2);
  EXPECT_EQ(mmf->view_size(), kPayloadSize - kGranularity * 2);
  EXPECT_TRUE(mmf->Read(kPayloadSize - 1, 2).empty());
}

TEST_F(MemoryMappedFileTest, ReMapRejectsAnOffsetPastTheEndOfTheFile) {
  ASSERT_TRUE(mmf->Map());
  EXPECT_FALSE(mmf->ReMap(kPayloadSize + kGranularity, kGranularity));
  // A rejected ReMap leaves no view behind.
  EXPECT_EQ(mmf->view_size(), 0u);
  EXPECT_TRUE(mmf->Read(0, 1).empty());
}

TEST_F(MemoryMappedFileTest, ReMapWithALengthThatWrapsIsRejected) {
  ASSERT_TRUE(mmf->Map());
  // offset + mapped_bytes wraps, which used to skip the clamp and ask the
  // kernel to map a length past the end of the file.
  EXPECT_FALSE(mmf->ReMap(base::MinMax<u64>::max() - 3, 8));
  EXPECT_EQ(mmf->view_size(), 0u);
}

TEST_F(MemoryMappedFileTest, WriteLandsAtTheFileOffsetItNames) {
  ASSERT_TRUE(mmf->Map());
  ASSERT_TRUE(mmf->ReMap(kGranularity, kGranularity));

  byte payload[4] = {0xDE, 0xAD, 0xBE, 0xEF};
  ASSERT_TRUE(mmf->Write(kGranularity, base::Span<byte>(payload, 4)));

  const base::Vector<byte> data = mmf->Read(kGranularity, 4);
  ASSERT_EQ(data.size(), 4u);
  for (mem_size i = 0; i < 4; ++i)
    EXPECT_EQ(data[i], payload[i]);
}

}  // namespace base
