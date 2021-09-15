// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "address_book_data.h"

namespace {
constexpr char kSyncDataName[] = "$ retk_address_book_data";
constexpr int kSessionStoreVersion = 1;
}  // namespace

const char* const AddressBookData::GetName() {
  return kSyncDataName;
}

void AddressBookData::Load() {

}

void AddressBookData::Save() {
}