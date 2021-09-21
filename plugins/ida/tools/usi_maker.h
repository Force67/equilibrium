// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class USIMaker {
 public:
  USIMaker() = default;

  using HashType = uint64_t;

  // Default init or failure
  static constexpr uint64_t kInvalidUSI = UINT64_MAX;

  HashType Create(const ea_t);
  bool CreateStore(const ea_t);

 private:
  char* utf8_storage_name_;
};