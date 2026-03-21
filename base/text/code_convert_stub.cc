// Stub implementation of WideToUTF8 for platforms where the full ICU-based
// code_convert.cc is not available. Handles basic ASCII/Latin-1 range only.
#include <base/export.h>
#include <base/strings/xstring.h>
#include <base/strings/string_ref.h>

namespace base {

BASE_EXPORT bool WideToUTF8(const wchar_t* src, mem_size src_len, base::String* output) {
  if (!output) return false;
  output->clear();
  for (mem_size i = 0; i < src_len; ++i) {
    wchar_t c = src[i];
    if (c < 0x80) {
      output->push_back(static_cast<char>(c));
    } else {
      // Simple fallback: replace non-ASCII with '?'
      output->push_back('?');
    }
  }
  return true;
}

[[nodiscard]] BASE_EXPORT base::StringU8 WideToUTF8(const base::StringRefW wide) {
  base::StringU8 result;
  for (mem_size i = 0; i < wide.size(); ++i) {
    wchar_t c = wide.data()[i];
    if (c < 0x80) {
      result.push_back(static_cast<char8_t>(c));
    } else {
      result.push_back(u8'?');
    }
  }
  return result;
}

}  // namespace base
