// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// Licensing data.
#pragma once

#include <base/arch.h>

namespace entitlement {
using namespace arch_types;

class License {
 public:

 private:
  u64 issue_date_;
  u64 expiry_date_;

};
}  // namespace entitelement