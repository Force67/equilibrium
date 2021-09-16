// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "usi_maker.h"
#include <typeinf.hpp>

namespace {
using hash_type = uint64_t;
constexpr hash_type fnv_basis = 14695981039346656037ull;
constexpr hash_type fnv_prime = 1099511628211ull;

// FNV-1a 64 bit hash
constexpr hash_type HashTypeName(const char* str, hash_type hash = fnv_basis) {
  return *str ? HashTypeName(str + 1, (hash ^ hash_type(*str)) * fnv_prime)
              : hash;
}


}  // namespace

// TODO: add prefix support..
uint64_t CreateUSI(const ea_t ea) {
  if (!get_func(ea))
    return kInvalidUSI;

  // extract function name
  qstring symbol_name;
  get_func_name(&symbol_name, ea);

  const size_t pos = symbol_name.find("sub_");
  if (pos != qstring::npos && pos == 0) {
    LOG_ERROR("Cant create USI for unnamed sub! ({})", symbol_name.c_str());
    return kInvalidUSI;
  }

  symbol_name += "(";

  // extract arguments
  tinfo_t info;
  get_tinfo(&info, ea);

  func_type_data_t fi;
  info.get_func_details(&fi);

  for (const funcarg_t& it : fi) {
    symbol_name += it.name;
  }

  symbol_name += ")";

  const uint64_t hash = HashTypeName(symbol_name.c_str());
  LOG_INFO("CreateUSI() -> {}=0x{:x}", symbol_name.c_str(), hash);

  // TODO: add function comment...

  return hash;
}

bool CreateReflectionSymbolTableEntry() {
  // TODO: fetch from storage.
  return true;
}