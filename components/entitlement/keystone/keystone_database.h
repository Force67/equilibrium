// Copyright (C) 2022 Vincent Hengel.
#pragma once

#include <base/containers/vector.h>
#include <base/strings/string_ref.h>
#include <base/time/time.h>
#include <entitlement/keystone/license_info_spec.h>

namespace entitlement {

// a keystone represents a license with it's set of capeabilities.
class KeyStoneDatabase {
 public:
  // start the license expiry
  void BeginOwnership();

  void SetLicensee(const base::StringRefU8 name) { licensee_ = name.c_str(); }

  void ForgeHeader();

  // add entitlement for a feature to the license
  void AddEntitlement(const char* license_fancy_id,
                      u64 feature_id,
                      base::time_type expiry_ms);

  void FixAndAdjustTimestamps();

  const LicenseHeader& header() { return header_; }
  const base::StringU8& licensee() { return licensee_; }
  const auto& entitlements() { return entilements_; }

 private:
  LicenseHeader header_;
  base::StringU8 licensee_;
  base::Vector<FeatureEntilement> entilements_;

 private:
  base::time_type start_timestamp_;
};
}  // namespace entitlement