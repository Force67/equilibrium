// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/filesystem/file.h>

#include <program_database/database_spec.h>
#include <program_database/database_writer.h>

namespace program_database {
constexpr u32 kDbFormatVersion = 1;

class FileWriter {
 public:
  FileWriter(base::File& f) : file_(f) {}

  template <typename T>
  void Write(const T& data) {
    file_.WriteAtCurrentPos(reinterpret_cast<const char*>(&data), sizeof(T));
  }

 private:
  base::File& file_;
};

void CreateDatabase(const base::Span<byte> program, const base::Path& out_path) {
  base::File f(out_path, base::File::FLAG_OPEN | base::File::FLAG_READ);

  FileWriter writer(f);
  const v1::Header db_header{.magic = v1::kMainHeaderMagic,
                             .db_version = kDbFormatVersion,
                             .user_id = 1337,
                             .retk_version = 1337,
                             .create_date_time_stamp = 0,
                             .last_modified_time_stamp = 0,
                             .progarm_seg_offset = 0,
                             .section_header_offset = 0};

  const v1::SegmentHeader section_header{.num_sections = 1337};
  writer.Write(db_header);
  writer.Write(section_header);


}
}  // namespace program_database
