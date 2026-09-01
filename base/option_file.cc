// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.

#include <base/option_file.h>

#include <base/containers/vector.h>
#include <base/filesystem/file.h>
#include <base/memory/unique_pointer.h>
#include <base/option.h>

namespace base {
namespace {

// Option<const char*> binds to the string it is handed instead of copying it,
// so a value parsed out of a file has to outlive that file's buffer. The pool
// owns one heap String per assignment; UniquePointer keeps the String itself
// put when the vector grows.
Vector<UniquePointer<String>>& ValuePool() {
  static Vector<UniquePointer<String>> pool;
  return pool;
}

const char* InternValue(const StringRef value) {
  auto owned = MakeUnique<String>(value.data(), value.length());
  const char* stable = (*owned).c_str();
  ValuePool().push_back(move(owned));
  return stable;
}

bool IsSpace(const char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

StringRef Trim(const StringRef text) {
  mem_size begin = 0, end = text.length();
  while (begin < end && IsSpace(text.data()[begin])) ++begin;
  while (end > begin && IsSpace(text.data()[end - 1])) --end;
  return text.subslice(begin, end - begin);
}

StringRef Unquote(const StringRef value) {
  if (value.length() < 2) return value;
  const char first = value.data()[0];
  if ((first == '"' || first == '\'') && value.data()[value.length() - 1] == first)
    return value.subslice(1, value.length() - 2);
  return value;
}

bool IsComment(const StringRef line) {
  if (line.data()[0] == '#') return true;
  return line.length() >= 2 && line.data()[0] == '/' && line.data()[1] == '/';
}

bool Matches(const OptionBase* option, const StringRef name) {
  if (option->name() && name == StringRef(option->name())) return true;
  return option->env() && name == StringRef(option->env());
}

// One entry, already trimmed and known non-empty.
void ApplyEntry(const StringRef entry, OptionApply apply,
                OptionFileResult& result) {
  StringRef body = entry;
  bool reset = false;
  if (body.data()[0] == '+') {
    body = body.subslice(1, body.length() - 1);
  } else if (body.data()[0] == '-') {
    body = body.subslice(1, body.length() - 1);
    reset = true;
  }

  const mem_size assign = body.find('=');
  StringRef name = Trim(assign == StringRef::npos ? body : body.subslice(0, assign));
  if (name.empty()) {
    ++result.invalid;
    return;
  }

  // A bare name is only an entry when it carries the +/- prefix that says what
  // to do with it; anything else on its own line is a typo, not an assignment.
  if (assign == StringRef::npos && body.length() == entry.length()) {
    ++result.invalid;
    return;
  }

  const StringRef value =
      assign == StringRef::npos
          ? StringRef("1")
          : Unquote(Trim(body.subslice(assign + 1, body.length() - assign - 1)));
  // A name may be declared in more than one place, once per module that reads
  // it; an entry means all of them, not whichever registered first.
  const char* stable = reset ? nullptr : InternValue(value);
  mem_size matched = 0, failed = 0, held = 0;
  OptionBase::VisitAll([&](const OptionBase* registered) {
    auto* option = const_cast<OptionBase*>(registered);
    if (!Matches(option, name)) return;
    ++matched;
    if (apply == OptionApply::kFillUnset && option->overridden()) {
      ++held;
      return;
    }
    if (reset)
      option->Reset();
    else if (!option->SetFromString(stable))
      ++failed;
  });

  if (!matched)
    ++result.unknown;
  else if (failed)
    ++result.invalid;
  else if (held == matched)
    ++result.skipped;
  else
    ++result.applied;
}

}  // namespace

OptionBase* FindOption(const StringRef name) {
  OptionBase* found = nullptr;
  OptionBase::VisitAll([&found, name](const OptionBase* registered) {
    // The chain stores const pointers; the options themselves are mutable
    // globals, so handing back a writable one is well-defined.
    auto* option = const_cast<OptionBase*>(registered);
    if (!found && Matches(option, name)) found = option;
  });
  return found;
}

bool SetOptionValue(const StringRef name, const StringRef value) {
  const char* stable = InternValue(value);
  bool set = false;
  OptionBase::VisitAll([&](const OptionBase* registered) {
    auto* option = const_cast<OptionBase*>(registered);
    if (Matches(option, name) && option->SetFromString(stable)) set = true;
  });
  return set;
}

OptionFileResult ApplyOptionText(const StringRef text,
                                 const OptionApply apply) {
  OptionFileResult result;
  result.read = true;

  mem_size begin = 0;
  while (begin <= text.length()) {
    mem_size end = begin;
    while (end < text.length() && text.data()[end] != '\n') ++end;

    const StringRef line = Trim(text.subslice(begin, end - begin));
    if (!line.empty() && !IsComment(line)) ApplyEntry(line, apply, result);

    if (end >= text.length()) break;
    begin = end + 1;
  }
  return result;
}

OptionFileResult ApplyOptionFile(const Path& path, const OptionApply apply) {
  i64 size = 0;
  auto content = ReadFile(path, &size);
  if (!content) return OptionFileResult{};
  if (size <= 0) {
    OptionFileResult empty;
    empty.read = true;
    return empty;
  }
  return ApplyOptionText(StringRef(reinterpret_cast<const char*>(&content[0]),
                                   static_cast<mem_size>(size)),
                         apply);
}

void AppendOptionText(String& out, const bool overridden_only) {
  char value[512];
  OptionBase::VisitAll([&out, &value, overridden_only](const OptionBase* option) {
    if (overridden_only && !option->overridden()) return;
    if (option->desc() && *option->desc()) {
      out.append("// ");
      out.append(option->desc());
      out.push_back('\n');
    }
    option->FormatValue(value, sizeof(value));
    out.push_back('+');
    out.append(option->name());
    out.push_back('=');
    out.append(value);
    out.push_back('\n');
  });
}

bool WriteOptionFile(const Path& path, const bool overridden_only) {
  String text;
  AppendOptionText(text, overridden_only);
  return WriteFile(path, reinterpret_cast<const byte*>(text.c_str()),
                   static_cast<i64>(text.length()));
}

}  // namespace base
