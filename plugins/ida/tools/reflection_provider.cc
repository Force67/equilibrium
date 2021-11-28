// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#if 0
#include "reflection_provider.h"
#include "signature_maker.h"

#include <typeinf.hpp>
#include <base/hash/fnv1a.h>
#include <utils/ida_plus.h>
#include <base/check.h>

// qstring.rfind doesn't confirm to standard behavior spec.
static size_t ReverseMatch(const qstring& str, char needle, size_t pos) {
  while (pos >= 0) {
    if (str[pos] == needle)
      return pos;
    else
      pos--;
  }
  return qstring::npos;
}

static void FormatTypeName(qstring& type_name) {
  // The bit containing the brackets must be the function, everything else
  // before the return value, scrub the return value.
  size_t pos = type_name.find('(');
  if (pos != qstring::npos) {
    pos = ReverseMatch(type_name, ' ', pos);
    if (pos != qstring::npos) {
      // +1 for length of ' '
      type_name = type_name.substr(pos + 1, type_name.length());
    }
  }
}

ReflectionProvider::ReflectionProvider(const char* path) : database_(path) {}

bool ReflectionProvider::AddSymbol(const ea_t ea) {
  using namespace tilted_reflection;

  qstring type_name;
  if (!print_type(&type_name, ea, PRTYPE_CPP)) {
    LOG_ERROR(
        "ReflectionProvider::AddSymbol()-> Failed to get type name for ea "
        "0x{:x}",
        ea);
    return false;
  }

  SymbolDatabase::Record record;
  SignatureMaker sigmaker;
  bool is_data = false;
  auto result = sigmaker.CreateSignature(
      ea, record.signature, record.direct_offset, is_data);

  if (result != SignatureMaker::Result::kSuccess) {
    LOG_ERROR(
        "ReflectionProvider::AddSymbol()-> Failed to create signature ({})",
        static_cast<int>(result));
    return false;
  }

  FormatTypeName(type_name);
  record.hash = base::FNV1a64(type_name.c_str());
  record.count = 1;
  record.indirect_offset = 0;
  database_.AddSymbol(record);

  // for now we save on every symbol, this is less than optimal however
  // TODO: think about recovery strategy if this fails.
  return database_.StoreSymbols();
}

void ReflectionProvider::HandleEvent(ui_notification_t code, va_list args) {
  if (code == ui_notification_t::ui_database_inited) {
    // Execute read in IDA thread to avoid lagging ui components.
    // UNTESTED
    utils::RequestFunctor<void()> functor([&]() { database_.LoadSymbols(); },
                                          MFF_READ);

    TK_UNUSED(functor);
  }
}
#endif