// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/containers/span.h>
#include <base/containers/vector.h>

namespace crypto {

class PublicKeyWrap;
class EntropyWrap;
class CrtDrbgWrap;

bool RSASign(PublicKeyWrap&,
             EntropyWrap&,
             CrtDrbgWrap&,
             const base::Span<byte> input,
             base::Vector<byte>& output);

bool RSAVerify(PublicKeyWrap&,
               const base::Span<byte> in_data,
               const base::Span<byte> signature);
}  // namespace crypto