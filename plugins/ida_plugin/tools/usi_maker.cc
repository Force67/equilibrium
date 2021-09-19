// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "usi_maker.h"

#include <optional>
#include <typeinf.hpp>

namespace {

// FNV-1a 64 bit hash
using hash_type = USIMaker::HashType;
constexpr hash_type fnv_basis = 14695981039346656037ull;
constexpr hash_type fnv_prime = 1099511628211ull;
constexpr hash_type HashTypeName(const char* str, hash_type hash = fnv_basis) {
  return *str ? HashTypeName(str + 1, (hash ^ hash_type(*str)) * fnv_prime)
              : hash;
}

std::optional<qstring> AssembleFullTypeName(const ea_t ea) {
  // extract function name
  qstring symbol_name;
  get_func_name(&symbol_name, ea);

  const size_t pos = symbol_name.find("sub_");
  if (pos != qstring::npos && pos == 0)
    return std::nullopt;

  symbol_name += "(";
  // append arguments:
  tinfo_t info;
  if (!get_tinfo(&info, ea))
    return std::nullopt;

  func_type_data_t fi;
  info.get_func_details(&fi);

  //calc_c_cpp_name()

  for (const funcarg_t& it : fi) {
    // for future reference:
    // this prints the current register name:
    // print_argloc(nameBuf, sizeof(nameBuf), it.argloc);
    qstring cpp_arg_type;
    if (!print_tinfo(&cpp_arg_type, nullptr, 0, 2, 16, &it.type, nullptr,
                     nullptr))
      return std::nullopt;

    symbol_name += ",";
    symbol_name += cpp_arg_type;
  }

  symbol_name += ")";
  return symbol_name;
}

// qstring.rfind doesn't confirm to standard behavior spec.
size_t ReverseMatch(const qstring& str, char needle, size_t pos) {
  while (pos >= 0) {
    if (str[pos] == needle)
      return pos;
    else
      pos--;
  }
  return qstring::npos;
}

void CleanCXXTypeName(qstring& type_name) {
  // The bit containing the brackets must be the function, everything else before
  // the return value, scrub the return value.
  size_t pos = type_name.find('(');
  if (pos != qstring::npos) {
    pos = ReverseMatch(type_name, ' ', pos);
    if (pos != qstring::npos) {
      // +1 for length of ' '
      type_name = type_name.substr(pos + 1, type_name.length());
    }
  }
}
}  // namespace


USIMaker::HashType USIMaker::Create(const ea_t ea) {
  // Why are we using print_type now? Well it works with anything
  // not just functions and handles all edge cases.
  qstring type_name;
  if (!print_type(&type_name, ea, PRTYPE_CPP))
    return kInvalidUSI;

  CleanCXXTypeName(type_name);
  // TODO: maybe apply function comment when its marked...
  const uint64_t hash = HashTypeName(type_name.c_str());
  LOG_INFO("CreateUSI() -> {}=0x{:x}", type_name.c_str(), hash);
  return hash;
}

bool USIMaker::CreateStore(const ea_t ea) {
  return false;
}