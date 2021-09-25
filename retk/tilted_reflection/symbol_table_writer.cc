// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "symbol_table_writer.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

// USI mangle

namespace tilted_reflection {
using namespace rapidjson;

namespace {
// FNV-1a 64 bit hash
using hash_type = SymbolTableWriter::HashType;
constexpr hash_type fnv_basis = 14695981039346656037ull;
constexpr hash_type fnv_prime = 1099511628211ull;
constexpr hash_type HashTypeName(const char* str, hash_type hash = fnv_basis) {
  return *str ? HashTypeName(str + 1, (hash ^ hash_type(*str)) * fnv_prime)
              : hash;
}

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
}  // namespace

SymbolTableWriter::SymbolTableWriter(){};
SymbolTableWriter::~SymbolTableWriter(){};

bool SymbolTableWriter::Open(const char* const path) {
  doc_ = std::make_unique<Document>();
  if (doc_->Parse("").HasParseError())
    return false;

  return true;
}

void SymbolTableWriter::JustCreate() {
  doc_ = std::make_unique<Document>();
  doc_->SetObject();
  doc_->AddMember("version", 1337, doc_->GetAllocator());

  list_ = std::make_unique<Value>(kArrayType);

}

std::string SymbolTableWriter::DocToString() {
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  doc_->Accept(writer);

  return buffer.GetString();
}

void SymbolTableWriter::CreateEntry(const char* pretty_name,
                                    const char* symbol_name,
                                    const CreateInfo& info) {
  // To Find out more about the symbol table spec, consult:
  // https://github.com/tiltedphoques/TiltedReflection/blob/questionable-fixes/reflection/SymbolTable.cpp
  // A record consists of:
  //	hash
  //	name
  //	offset
  //	count
  //	code-sig
  //	directive
  Value ovr(kArrayType);
  auto& allocator = doc_->GetAllocator();

  Value json_entry(kObjectType);
  json_entry.AddMember("hash", HashTypeName(symbol_name), allocator);
  AddStr(*doc_, pretty_name, json_entry, "name");

  json_entry.AddMember("offset", info.direct_offset, allocator);
  json_entry.AddMember("count", info.count, allocator);

  Value str(kObjectType);
  AddStr(*doc_, info.signature, json_entry, "code-sig");

  if (info.indirect_offset) {
    switch (info.kind) {
      case Kind::kFunction:
      case Kind::kMemberFunction: {
        json_entry.AddMember("get-call", info.indirect_offset, allocator);
        break;
      }
      // TODO: evaluate if this really makes sense to do.
      case kVariable: {
        json_entry.AddMember("get-address", info.indirect_offset, allocator);
        break;
      }
    }
  }

  ovr.PushBack(json_entry, allocator);
  doc_->AddMember("override-list", ovr, allocator);
}
}  // namespace tilted_reflection