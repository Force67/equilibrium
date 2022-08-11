// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing data.

#include <entitlement/license.h>
#include <entitlement/mbed_raii_wrappers.h>
#include <base/containers/vector.h>
#include <base/math/math_helpers.h>

namespace entitlement {
namespace {
constexpr const char8_t kIssuingAuthority[]{u8"VH-TECH"};

template <typename T>
mem_size WritePaddedString(u8* head, const base::BasicStringRef<T> ref) {
  *reinterpret_cast<mem_size*>(head) = ref.length();
  std::memcpy(head + sizeof(mem_size), ref.c_str(), ref.length());

  // write 0 padding.
  const mem_size data_size = sizeof(mem_size) + ref.length();
  const mem_size pad = base::NextPowerOf2(data_size);
  std::memset(head + data_size, 0, pad);
  return data_size + pad;
}
}  // namespace

base::String EncodeAndSignLicenseBlock(
    const LicenseHeader& header,
    const base::StringU8& licensee_name,
    const base::StringRefU8 optional_additional_data) {
  // Should use some bufferdgrowbuffer class....
  base::Vector<u8> buffer(2048, base::VectorReservePolicy::kForData);

  *reinterpret_cast<LicenseHeader*>(buffer.data()) = header;
  auto offset = WritePaddedString<char8_t>(buffer.data() + sizeof(LicenseHeader),
                                           kIssuingAuthority);
  offset += WritePaddedString<char8_t>(buffer.data() + offset, licensee_name);
  offset +=
      WritePaddedString<char8_t>(buffer.data() + offset, optional_additional_data);

  base::String base64_result;
  int result = Base64Encode(base64_result, base::Span(buffer.data(), buffer.size()));

  // begin base64
  u8 signature[32];  //< Signature to prevent alteration
  // end base64
}
}  // namespace entitlement