// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/logging.h>
#include <base/filesystem/file.h>
#include <program_database/database_reader.h>
#include <program_database/database_constants.h>

namespace program_database {
namespace {

class DiskFileReader {
 public:
  DiskFileReader(base::File& f) : file_(f) {}

  template <typename T>
  i32 Read(T& data) {
    i32 bytes_read =
        file_.ReadAtCurrentPos(reinterpret_cast<char*>(&data), sizeof(T));

    if (bytes_read !=
        sizeof(T)) {
      LOG_ERROR("Bad read of {} bytes at pos {}", sizeof(T), 1337);
      return 0;
    }

    return bytes_read;
  }

  template<typename T>
  i32 Read(T* data, mem_size size) {
    i32 bytes_read = file_.ReadAtCurrentPos(reinterpret_cast<char*>(data), size);

    if (bytes_read != sizeof(T)) {
      LOG_ERROR("Bad read of {} bytes at pos {}", size, 1337);
      return 0;
    }

    return bytes_read;
  }

  void SetPos(i64 ofs) { 
      auto r = file_.Seek(base::File::Whence::FROM_BEGIN, ofs);
    __debugbreak();
  }

 private:
  base::File& file_;
};
}  // namespace

bool LoadIntoSymbolTable(SymbolTable* symbol_table,
                         const base::Path& database_file_path) {
  base::File f(database_file_path, base::File::FLAG_OPEN | base::File::FLAG_READ |
                                       base::File::FLAG_WRITE);
  if (!f.IsValid())
    return false;

  DiskFileReader reader(f);

  u32 magic = 0;
  reader.Read<u32>(magic);
  if (magic != v1::kMainHeaderMagic) {
    LOG_ERROR("Failed to load database: invalid magic");
    return false;
  }

  u16 current_v = 0;
  reader.Read<u16>(current_v);
  if (current_v > kCurrentTkDbVersion) {
    LOG_ERROR(
        "Database version is newer than expected. database version: v{} current "
        "version is {}",
        current_v, kCurrentTkDbVersion);
    return false;
  }

  reader.SetPos(0);

  auto header_data{std::make_unique<byte>()};
  reader.Read(header_data.get(), v1::kSizeOfHeaders);
  // this steals the header data, dont use from now
  symbol_table->LoadHeaders(std::move(header_data));

  // we should now be at segment 0 offset..


  return true;
}
}  // namespace program_database