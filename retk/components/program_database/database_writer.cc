// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/filesystem/file.h>

#include <base/time/time.h>
#include <base/math/alignment.h>
#include <base/math/math_helpers.h>
#include <base/memory/memory_literals.h>
#include <program_database/database_writer.h>

namespace program_database {
namespace {
using namespace base::memory_literals;

constexpr u32 kDbPageSize = 64_kib;
constexpr u16 kSectionAlignment = 16;

u16 EstimateInitialPageCount(base::Span<byte> program) {
  // TODO: we should keep stats on this.. this is not a very precise measurement
  // estimate pagecount by going off the program size
  return program.size() < kDbPageSize
             ? 2
             : static_cast<u16>(ceil(program.size() / kDbPageSize));
}
}  // namespace

class DiskFileWriter {
 public:
  DiskFileWriter(base::File& f) : file_(f) {}

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
    std::memset(x.get(), 0, n);

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

bool WriteInitialDiskFile(const base::Span<byte> program,
                          const base::Path& out_path,
                          const u32 retk_version,
                          const u32 user_id) {
  base::File f(out_path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);

  DCHECK(f.IsValid());
  if (!f.IsValid())
    return false;

  i64 now_timestamp = base::GetUnixTimeStamp();

  constexpr u32 past_headers = sizeof(v1::Header) + sizeof(v1::SegmentHeader);
  constexpr u32 seg_0_offset =
      past_headers + base::NextPowerOf2_Compile(past_headers) - past_headers;

  DiskFileWriter writer(f);
  const v1::Header db_header{.magic = v1::kMainHeaderMagic,
                             .create_version = v1::kCurrentTkDbVersion,
                             .current_version = v1::kCurrentTkDbVersion,
                             .user_id = user_id,
                             .retk_version = retk_version,
                             .create_date_time_stamp = now_timestamp,
                             .last_modified_time_stamp = now_timestamp,
                             .seg_0_offset = seg_0_offset,
                             .section_header_offset = sizeof(v1::Header)};
  const u16 page_count_initial = EstimateInitialPageCount(program);
  const v1::SegmentHeader section_header{
      .num_sections = page_count_initial,
      .section_alignment = kSectionAlignment,
      .page_size = kDbPageSize,
  };
  u32 pos = 0;
  pos += writer.Write(db_header);       // +40
  pos += writer.Write(section_header);  // +8
  pos += writer.WritePad(base::NextPowerOf2(pos) - pos);

  BUGCHECK(pos == seg_0_offset, "Headers have been written improperly");

  // TODO: after this we must memory store...

  // write out page 1
  const v1::SourceProgramHeader program_desc{
      .magic = v1::kProgramHeaderMagic,
      .compression = v1::CompressionType::kNone,
      .name = {"none"},
      .program_size = program.size()};

  pos += writer.Write(program_desc);
  pos += writer.WritePad(base::NextPowerOf2(pos) - pos);
  pos += writer.WriteSZ(*program.data(), program.size());
  pos += writer.WritePad(kDbPageSize - pos);
  // lay out initial page set
  for (i32 i = 1; i < page_count_initial; i++) {
    writer.WritePad(kDbPageSize);
  }

  return true;
}
}  // namespace program_database
