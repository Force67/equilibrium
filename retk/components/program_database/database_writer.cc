// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/filesystem/file.h>

#include <base/math/alignment.h>
#include <base/math/math_helpers.h>
#include <base/memory/memory_literals.h>

#include <program_database/database_spec.h>
#include <program_database/database_writer.h>

namespace program_database {
using namespace base::memory_literals;

constexpr u32 kDbFormatVersion = 1;
constexpr u32 kDbPageSize = 64_kib;

class FileWriter {
 public:
  FileWriter(base::File& f) : file_(f) {}

  template <typename T>
  i32 Write(const T& data) {
    return file_.WriteAtCurrentPos(reinterpret_cast<const char*>(&data), sizeof(T));
  }

  i32 WritePad(mem_size n) {
    u64 pad_count = n / sizeof(u64);
    u64 qword = 0;

    // this is the shittiest way of doing this
    // but i am tired today
    auto x{std::make_unique<byte[]>(n)};

    return file_.WriteAtCurrentPos(reinterpret_cast<const char*>(x.get()), n);
  }

  template <typename T>
  i32 WriteSZ(const T& var, mem_size n) {
    return file_.WriteAtCurrentPos(reinterpret_cast<const char*>(&var), n);
  }

  void SetPos(i64 ofs) { file_.Seek(base::File::Whence::FROM_BEGIN, ofs); }

 private:
  base::File& file_;
};

void WriteInitialDiskFile(const base::Span<byte> program,
                          const base::Path& out_path,
                          const u32 retk_version,
                          const u32 user_id) {
  base::File f(out_path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  DCHECK(f.IsValid());

  FileWriter writer(f);
  const v1::Header db_header{.magic = v1::kMainHeaderMagic,
                             .db_version = kDbFormatVersion,
                             .user_id = user_id,
                             .retk_version = retk_version,
                             .create_date_time_stamp = 0,
                             .last_modified_time_stamp = 0,
                             .program_seg_offset = 0,
                             .section_header_offset = 0};
  // TODO: we should keep stats on this.. this is not a very precise measurement
  // estimate pagecount by going off the program size
  const u16 initial_pc = program.size() < kDbPageSize
                             ? 2
                             : static_cast<u16>(ceil(program.size() / kDbPageSize));
  const v1::SegmentHeader section_header{
      .num_sections = initial_pc,
      .section_alignment = 64,
      .page_size = kDbPageSize,
  };
  u32 pos = 0;
  pos += writer.Write(db_header);       // +40
  pos += writer.Write(section_header);  // +8
  pos += writer.WritePad(base::NextPowerOf2(pos) - pos);

  // write out page 1
  const v1::SourceProgramHeader program_desc{
      .magic = v1::kProgramHeaderMagic,
      .compression = v1::CompressionType::kNone,
      .name = {"none"},
      .program_size = program.size()};
  pos += writer.Write(program_desc);
  // TODO: pad inbetween
  pos +=
      writer.WriteSZ(*program.data(), program.size());
  pos += writer.WritePad(kDbPageSize - pos);  // writen doesnt write multiple n

  // lay out initial page set

  for (i32 i = 0; i < initial_pc - 1; i++) {
    writer.WritePad(kDbPageSize);
  }
}
}  // namespace program_database
