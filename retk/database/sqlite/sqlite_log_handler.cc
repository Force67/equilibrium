// Copyright 2021 (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// This defines the sqlite3_init_sqllog symbol globally so we get our log initialized
// when sqlite initializes.
#include <base/logging.h>
#include <sqlite3rc.h>

void sqlite3_init_sqllog() {
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

  LOG_INFO("Initializing sqlite3 version " TOSTRING(SQLITE_RESOURCE_VERSION));
#undef STRINGIFY
#undef TOSTRING
}