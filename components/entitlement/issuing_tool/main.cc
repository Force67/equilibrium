// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/filesystem/path.h>
#include <entitlement/keystone/keystone_database.h>
#include <entitlement/issuing_tool/license_file_writer.h>

int main() {
  // create a keystone for the user.
  entitlement::KeyStoneDatabase ks;
  ks.SetLicensee(u8"Vince Van Gogh");

  ks.BeginOwnership();
  ks.ForgeHeader();
  ks.AddEntitlement("AAA-BBB-CCC", 13371337, 10000);

  entitlement::issuing_tool::WriteAndFormatLicenseFile(R"(Q:\Mods\retk.lk)", ks);

  return 0;
}