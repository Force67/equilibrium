// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <gtest/gtest.h>

#include <base/filesystem/file.h>
#include <base/filesystem/file_util.h>
#include <base/filesystem/scoped_temp_dir.h>

namespace {

// Creates an empty regular file, so the delete tests have something to remove.
bool Touch(const base::Path& path) {
  base::File file(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  return file.IsValid();
}

TEST(FileUtilDeleteTest, DeletesARegularFile) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  const base::Path file = dir.path() / "victim.txt";
  ASSERT_TRUE(Touch(file));
  ASSERT_TRUE(base::PathExists(file));

  EXPECT_TRUE(base::DeletePathRecursively(file));
  EXPECT_FALSE(base::PathExists(file));
}

TEST(FileUtilDeleteTest, DeletingAMissingPathSucceeds) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());

  // Already gone is the state the caller asked for.
  EXPECT_TRUE(base::DeletePathRecursively(dir.path() / "never_existed"));
}

TEST(FileUtilDeleteTest, RemovesAnEmptyDirectory) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  const base::Path empty = dir.path() / "empty";
  ASSERT_TRUE(base::CreateDirectory(empty));

  EXPECT_TRUE(base::DeletePathRecursively(empty));
  EXPECT_FALSE(base::DirectoryExists(empty));
}

TEST(FileUtilDeleteTest, RecursiveDeleteClearsANestedTree) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  const base::Path root = dir.path() / "tree";
  const base::Path nested = root / "a" / "b";
  ASSERT_TRUE(base::CreateDirectory(nested));
  ASSERT_TRUE(Touch(root / "top.txt"));
  ASSERT_TRUE(Touch(nested / "leaf.txt"));

  EXPECT_TRUE(base::DeletePathRecursively(root));
  EXPECT_FALSE(base::PathExists(root));
}

TEST(FileUtilDeleteTest, RecursiveDeleteOfASingleFile) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  const base::Path file = dir.path() / "lonely.txt";
  ASSERT_TRUE(Touch(file));

  EXPECT_TRUE(base::DeletePathRecursively(file));
  EXPECT_FALSE(base::PathExists(file));
}

TEST(ScopedTempDirTest, DeleteRemovesTheDirectoryAndForgetsIt) {
  base::ScopedTempDir dir;
  ASSERT_TRUE(dir.CreateUniqueTempDir());
  const base::Path path = dir.path();
  ASSERT_TRUE(Touch(path / "content.txt"));

  EXPECT_TRUE(dir.Delete());
  EXPECT_FALSE(base::DirectoryExists(path));
  EXPECT_FALSE(dir.IsValid());
}

TEST(ScopedTempDirTest, DestructorCleansUpEvenWithContent) {
  base::Path path;
  {
    base::ScopedTempDir dir;
    ASSERT_TRUE(dir.CreateUniqueTempDir());
    path = dir.path();
    ASSERT_TRUE(Touch(path / "content.txt"));
    ASSERT_TRUE(base::DirectoryExists(path));
  }

  EXPECT_FALSE(base::DirectoryExists(path));
}

}  // namespace
