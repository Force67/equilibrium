// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/filesystem/file.h>
#include <base/filesystem/file_util.h>
#include <base/filesystem/scoped_temp_dir.h>

namespace {

TEST(FileTests, Create) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  auto file_path = temp_dir.path() / "create_file_1";

  {
    // Don't create a File at all.
    base::File file;
    EXPECT_FALSE(file.IsValid());
    EXPECT_EQ(base::File::FILE_ERROR_FAILED, file.error_details());

    base::File file2(base::File::FILE_ERROR_TOO_MANY_OPENED);
    EXPECT_FALSE(file2.IsValid());
    EXPECT_EQ(base::File::FILE_ERROR_TOO_MANY_OPENED, file2.error_details());
  }

  {
    // Open a file that doesn't exist.
    base::File file(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
    EXPECT_FALSE(file.IsValid());
    EXPECT_EQ(base::File::FILE_ERROR_NOT_FOUND, file.error_details());
    EXPECT_EQ(base::File::FILE_ERROR_NOT_FOUND, base::File::GetLastFileError());
  }

  {
    // Open or create a file.
    base::File file(file_path, base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_READ);
    EXPECT_TRUE(file.IsValid());
    EXPECT_TRUE(file.created());
    EXPECT_EQ(base::File::FILE_OK, file.error_details());
  }

  {
    // Open an existing file.
    base::File file(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
    EXPECT_TRUE(file.IsValid());
    EXPECT_FALSE(file.created());
    EXPECT_EQ(base::File::FILE_OK, file.error_details());

    // This time verify closing the file.
    file.Close();
    EXPECT_FALSE(file.IsValid());
  }

  {
    // Open an existing file through Initialize
    base::File file;
    file.Initialize(file_path, base::File::FLAG_OPEN | base::File::FLAG_READ);
    EXPECT_TRUE(file.IsValid());
    EXPECT_FALSE(file.created());
    EXPECT_EQ(base::File::FILE_OK, file.error_details());

    // This time verify closing the file.
    file.Close();
    EXPECT_FALSE(file.IsValid());
  }

  {
    // Create a file that exists.
    base::File file(file_path, base::File::FLAG_CREATE | base::File::FLAG_READ);
    EXPECT_FALSE(file.IsValid());
    EXPECT_FALSE(file.created());
    EXPECT_EQ(base::File::FILE_ERROR_EXISTS, file.error_details());
    EXPECT_EQ(base::File::FILE_ERROR_EXISTS, base::File::GetLastFileError());
  }

  {
    // Create or overwrite a file.
    base::File file(file_path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    EXPECT_TRUE(file.IsValid());
    EXPECT_TRUE(file.created());
    EXPECT_EQ(base::File::FILE_OK, file.error_details());
  }

  {
    // Create a delete-on-close file.
    file_path = temp_dir.path() / "create_file_2";
    base::File file(file_path, base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_READ |
                             base::File::FLAG_DELETE_ON_CLOSE);
    EXPECT_TRUE(file.IsValid());
    EXPECT_TRUE(file.created());
    EXPECT_EQ(base::File::FILE_OK, file.error_details());
  }

  EXPECT_FALSE(base::PathExists(file_path));
}
}  // namespace