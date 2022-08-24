// Copyright (C) 2022 Vincent Hengel.

#include <base/time/time.h>
#include <entitlement/keystone/keystone_database.h>

namespace entitlement {
void KeyStoneDatabase::BeginOwnership() {
  start_timestamp_ = base::GetUnixTimeStamp();
}

void KeyStoneDatabase::FixAndAdjustTimestamps() {
  BeginOwnership();
  header_.issue_timestamp = start_timestamp_;
  header_.expiry_timestamp = 1337;
  for (FeatureEntilement& f : entilements_)
    f.support_expiry_timestamp = start_timestamp_ + 1000;
}

void KeyStoneDatabase::ForgeHeader() {
  header_ = {.version = 1,
            .type = LicenseHeader::Type::Regular,
            .size = sizeof(LicenseHeader),
            .reserved1 = 0,
             .issue_timestamp = start_timestamp_,
             .expiry_timestamp = start_timestamp_ + 1337};
}

void KeyStoneDatabase::AddEntitlement(const char* license_fancy_id,
                                      u64 feature_id,
                                      base::time_type expiry_ms) {
  FeatureEntilement fe{
      .version = 1,
      .reserved0 = 0,
      .license_id{},
      .feature_id = feature_id,
      .support_expiry_timestamp = start_timestamp_ + expiry_ms};
  std::strcpy(fe.license_id, license_fancy_id);

  entilements_.push_back(fe);
}
}  // namespace entitlement