// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "usi_maker.h"

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

  qstring function_name;
  get_func_name(&function_name, ea);

  //get_arg_addrs()

  const size_t pos = function_name.find("sub_");
  if (pos != qstring::npos && pos == 0) {
    LOG_ERROR("Cant create USI for unnamed sub! ({})", function_name.c_str());
    return kInvalidUSI;
  }


  const uint64_t hash = HashTypeName(function_name.c_str());
  LOG_INFO("CreateUSI() -> {}=0x{:x}", function_name.c_str(), hash);

  // TODO: add function comment...

  return hash;
}

bool CreateReflectionSymbolTableEntry() {
  // TODO: fetch from storage.
  return true;
}