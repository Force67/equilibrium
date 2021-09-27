// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "symbol_database.h"

#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/istreamwrapper.h>

#include <base/hash/fnv1a.h>
#include <base/logging.h>

// USI mangle

namespace tilted_reflection {
using namespace rapidjson;
namespace {
void AddStr(Document& doc,
            std::string_view str_view,
            Value& dest,
            std::string_view name) {
  auto& allocator = doc.GetAllocator();

  Value value(kObjectType);
  value.SetString(str_view.data(), static_cast<SizeType>(str_view.length()),
                  allocator);

  const auto string_ref = StringRef(name.data(), name.length());
  // Append it to source
  dest.AddMember(string_ref, value, allocator);
}

// To Find out more about the symbol table spec, consult:
// https://github.com/tiltedphoques/TiltedReflection/blob/questionable-fixes/reflection/SymbolTable.cpp
// A record consists of:
//	hash
//	name
//	offset
//	count
//	code-sig
//	directive

void WriteJSONEntry(Document& doc,
                    Value& ovr_list,
                    const SymbolDatabase::Record& r) {
  auto& allocator = doc.GetAllocator();

  Value json_entry(kObjectType);
  json_entry.AddMember("hash", r.hash, allocator);
  json_entry.AddMember("offset", r.direct_offset, allocator);
  json_entry.AddMember("count", r.count, allocator);
  AddStr(doc, r.signature, json_entry, "code-sig");
  if (r.indirect_offset)
    json_entry.AddMember("get-address", r.indirect_offset, allocator);

  ovr_list.PushBack(json_entry, allocator);
}

void ReadJSONEntry(Document& doc,
                   const Value& json_entry,
                   SymbolDatabase::Record& r) {
  r.hash = json_entry["hash"].GetUint64();
  r.direct_offset = json_entry["offset"].GetInt();
  r.count = json_entry["count"].GetUint();
  r.signature = json_entry["code-sig"].GetString();
  r.indirect_offset = json_entry["get-address"].GetInt();
}

// Keep this in sync with Reflection tool.
constexpr int kTableVersion = 2;
}  // namespace

SymbolDatabase::SymbolDatabase(const char* path) : path_(path){};
SymbolDatabase::~SymbolDatabase(){};

void SymbolDatabase::AddSymbol(const Record& record) {
  auto it =
      std::find_if(records_.begin(), records_.end(),
                   [&](const Record& r) { return r.hash == record.hash; });
  if (it == records_.end())
    records_.push_back(record);
}

//https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/clang/include/clang/Tooling/CompilationDatabase.h
//https://github.com/llvm/llvm-project/blob/d480f968ad8b56d3ee4a6b6df5532d485b0ad01e/clang/lib/Tooling/CompilationDatabase.cpp

bool SymbolDatabase::LoadSymbols() {
  TK_DCHECK(path_);

  std::ifstream ifs(path_);
  if (!ifs.good())
    return false;
  IStreamWrapper isw(ifs);

  Document doc;
  doc.ParseStream(isw);

  // No backwards compatibility guarantee at this time.
  if (doc["version"] != kTableVersion)
    return false;

  Value& list = doc["override-list"];
  TK_DCHECK(list.IsArray());

  records_.resize(list.Size());
  for (SizeType i = 0; i < list.Size(); i++) {
    ReadJSONEntry(doc, list[i], records_[i]);
  }

  return true;
}

bool SymbolDatabase::StoreSymbols() {
  TK_DCHECK(path_);

  Document doc;
  doc.SetObject();
  doc.AddMember("version", kTableVersion, doc.GetAllocator());

  Value list(kArrayType);
  for (const Record& r : records_) {
    WriteJSONEntry(doc, list, r);
  }
  doc.AddMember("override-list", list, doc.GetAllocator());

  // I know this sucks, at it needs a proper fix in the future
  std::ofstream ofs(path_);
  if (!ofs.good())
    return false;

  OStreamWrapper osw(ofs);
  Writer<OStreamWrapper> writer(osw);
  return doc.Accept(writer);
}
}  // namespace tilted_reflection