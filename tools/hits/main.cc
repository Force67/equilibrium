// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Utility for matching 'hits' of patterns.

#include <base/io/file/file.h>

class Executable {
 public:
  void Load(const base::FilePath& path) {
    base::File file(path, false);
    if (!file.IsValid()) {
      return;
    }

    size_t length = file.Seek(base::File::Whence::FROM_END, 0);
    ptr_ = std::make_unique<char[]>(length);
    file.Seek(base::File::Whence::FROM_BEGIN, 0);
    file.Read(0, ptr_.get(), length);
  }

  private:
  std::unique_ptr<char[]> ptr_;
};

int main(int argc, char** argv) {
  constexpr char kFileName[] =
      R"(C:\Users\vince\Documents\Development\Tilted\Reverse\diffing\test.txt)";

  const base::FilePath p(kFileName);

  Executable ex;
  ex.Load(p);

  return 0;
}