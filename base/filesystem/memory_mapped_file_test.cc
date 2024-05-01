#include <gtest/gtest.h>
#include <base/memory/unique_pointer.h>
#include "memory_mapped_file.h"
#include "scoped_temp_dir.h"

namespace base {

class MemoryMappedFileTest : public ::testing::Test {
 protected:
  base::UniquePointer<MemoryMappedFile> mmf;
  base::File temp_file;
  // base::ScopedTempDir temp_dir;

  void SetUp() override {
    // ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
    // auto path = temp_dir.path();
    // path.Append("temp_file");
    base::Path path("./temp_file");
    temp_file.Initialize(path, base::File::FLAG_CREATE_ALWAYS |
                                   base::File::FLAG_WRITE |
                                   base::File::FLAG_READ);
    ASSERT_TRUE(temp_file.IsValid());
    mmf = base::MakeUnique<MemoryMappedFile>(temp_file);
  }

  void TearDown() override {
    mmf.Reset();
    temp_file.Close();
  }
};

TEST_F(MemoryMappedFileTest, MapSuccess) {
  EXPECT_TRUE(mmf->Map());
}

TEST_F(MemoryMappedFileTest, ReMapValid) {
  mmf->Map();
  // Assuming the file size allows, remap to a different offset
  //EXPECT_TRUE(mmf->ReMap(100, 200));
}


#if 0
TEST_F(MemoryMappedFileTest, MapFailure) {
  // Set up a scenario where mapping should fail (e.g., using an invalid file)
  EXPECT_FALSE(mmf->Map());
}

TEST_F(MemoryMappedFileTest, ReadValid) {
  mmf->Map();
  auto data = mmf->Read(0, some_valid_size);
  EXPECT_EQ(data.size(), some_valid_size);
  // Additional validation on the data read
}

TEST_F(MemoryMappedFileTest, ReadInvalid) {
  mmf->Map();
  auto data = mmf->Read(invalid_offset, invalid_size);
  EXPECT_TRUE(data.empty());
}

TEST_F(MemoryMappedFileTest, WriteValid) {
  mmf->Map();
  base::Vector<byte> data_to_write = {/* initialize with test data */};
  EXPECT_TRUE(mmf->Write(0, data_to_write));
  // Read back data and validate
}

TEST_F(MemoryMappedFileTest, WriteInvalid) {
  mmf->Map();
  base::Vector<byte> data_to_write = {/* initialize with test data */};
  EXPECT_FALSE(mmf->Write(invalid_offset, data_to_write));
}


TEST_F(MemoryMappedFileTest, ReMapInvalid) {
  mmf->Map();
  // Attempt to remap beyond the file size
  EXPECT_FALSE(mmf->ReMap(file_size_too_large, 200));  // example values
}
#endif

}  // namespace base
