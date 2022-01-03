// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "address_book_data.h"

namespace {
constexpr char kSyncDataName[] = "$ retk_address_book_data";
constexpr int kSessionStoreVersion = 1;
}  // namespace

AddressBookData::AddressBookData() : IdbNode(kSyncDataName) {}

void AddressBookData::Load() {}

void AddressBookData::Save() {}