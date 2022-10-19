// Copyright (C) 2022 Vincent Hengel.
#pragma once

#include <base/containers/vector.h>
#include <base/containers/span.h>

#include <base/strings/string_ref.h>
#include <base/time/time.h>

#include <flatbuffers/flatbuffers.h>
#include <protocol/license_generated.h>

#include <base/memory/unique_pointer.h>

namespace entitlement {

class LicenseBuilder {
 public:
  LicenseBuilder(flatbuffers::FlatBufferBuilder& fbb)
      : fbb_(fbb) {
    start_timestamp_ = base::GetUnixTimeStamp();
  }

  static constexpr u16 kFeatureStructVersion = 1;
  static constexpr u16 kLicenseStructVersion = 1;

  void AddEntitlement(const char* license_fancy_id,
                      u64 feature_id,
                      base::time_type expiry_ms) {
    auto lid = fbb_.CreateString(license_fancy_id);

    entitlement::FeatureBuilder builder_(fbb_);
    builder_.add_support_expiry_timestamp(start_timestamp_ + 1337);
    builder_.add_feature_id(1337);
    builder_.add_license_id(lid);
    builder_.add_struct_version(kFeatureStructVersion);
    features_.push_back(builder_.Finish());
  }

  void BeginLicense() {
    auto feature_set = fbb_.CreateVector(features_.data(), features_.size());
    DCHECK(features_.size() > 0);

    license_builder_ = base::MakeUnique<LicenseBlockBuilder>(fbb_);
    license_builder_->add_entitlements(feature_set);
  }

  void SetLicensee(base::StringRef name) {
    DCHECK(!license_builder_);
    licensee_name_ = fbb_.CreateString(name);
  }
  void SetLicenseID(base::StringRef lid) {
    DCHECK(!license_builder_);
    licensee_id_ = fbb_.CreateString(lid);
  }
  void SetIssuingAuthority(base::StringRef authority) {
    DCHECK(!license_builder_);
    authority_name_ = fbb_.CreateString(authority);
  }
  void SetLicenseType(LicenseType lt) { license_builder_->add_type(lt); }

  void Finish() {
    license_builder_->add_licensee(licensee_name_);
    license_builder_->add_issuing_authority(authority_name_);
    license_builder_->add_struct_version(kLicenseStructVersion);
    license_builder_->add_issue_date(start_timestamp_);
    license_builder_->add_expiry_date(start_timestamp_ + 1000);
    license_builder_->add_license_id(licensee_id_);
    fbb_.Finish(license_builder_->Finish());
  }

 private:
  flatbuffers::FlatBufferBuilder& fbb_;
  flatbuffers::Offset<flatbuffers::String> licensee_name_;
  flatbuffers::Offset<flatbuffers::String> licensee_id_;
  flatbuffers::Offset<flatbuffers::String> authority_name_;
  base::UniquePointer<LicenseBlockBuilder> license_builder_{};
  base::time_type start_timestamp_;
  base::Vector<flatbuffers::Offset<entitlement::Feature>> features_;
};
}  // namespace entitlement