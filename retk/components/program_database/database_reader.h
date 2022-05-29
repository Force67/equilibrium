// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <base/filesystem/path.h>
#include <program_database/symbol_table.h>

namespace program_database {
bool LoadIntoSymbolTable(SymbolTable* symbol_table,
                         const base::Path& database_file_path);
}

// LoadIntoSymbolTable
// InitializeSymbolTableFromDatabase