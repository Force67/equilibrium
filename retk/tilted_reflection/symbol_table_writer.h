// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include <string>
#include <optional>
#include <rapidjson/document.h>

/* namespace rapidjson {
class Document;
}*/

namespace tilted_reflection {
class SymbolTableWriter final {
 public:
  SymbolTableWriter();
  ~SymbolTableWriter();

  using HashType = uint64_t;

  enum Kind { kFunction, kMemberFunction, kVariable };

  bool Open(const char* const path);
  void JustCreate();

  struct CreateInfo {
    int8_t direct_offset;
    int8_t indirect_offset;
    uint16_t count;
    std::string signature;
    Kind kind;
  };
  void CreateEntry(const char* pretty_name,
                   const char* symbol_name,
                   const CreateInfo&);

  std::string DocToString();

 private:
  std::unique_ptr<rapidjson::Document> doc_;
  std::unique_ptr<rapidjson::Value> list_;
  // table with list, that (re-writes) doc
};
}  // namespace tilted_reflection