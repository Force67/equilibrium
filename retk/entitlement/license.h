// Copyright (C) 2021 Force67 <github.com/Force67>.
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