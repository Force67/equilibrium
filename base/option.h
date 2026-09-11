// Copyright (C) 2026 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
// LLVM cl::opt-style global options. An Option<T> is a named global with a
// compiled-in default that a source (today: the environment) can override. Like
// base::Feature it self-registers through InitChain, so InitOptionsFromEnv()
// can find and populate every option without each site hand-rolling getenv.
#pragma once

#include <cstdlib>
#include <cstring>
#include <type_traits>

#include <base/arch.h>
#include <base/containers/init_chain.h>
#include <base/export.h>
#include <base/strings/format.h>

namespace base {
namespace detail {

// Parse `text` into `out`. Returns false (leaving `out` untouched) when the
// text is not a valid value for T. Covers bool, the integral and floating
// types, and const char* (which is bound to the source string, not copied).
template <typename T>
inline bool ParseOption(const char* text, T& out) {
  if constexpr (std::is_same_v<T, bool>) {
    if (!std::strcmp(text, "1") || !std::strcmp(text, "true") ||
        !std::strcmp(text, "on") || !std::strcmp(text, "yes")) {
      out = true;
      return true;
    }
    if (!*text || !std::strcmp(text, "0") || !std::strcmp(text, "false") ||
        !std::strcmp(text, "off") || !std::strcmp(text, "no")) {
      out = false;
      return true;
    }
    // Any other number counts: a knob set to 2 means on, as it would in a shell
    // test, not "unparsable".
    char* end = nullptr;
    const long long v = std::strtoll(text, &end, 0);
    if (end != text && !*end) {
      out = v != 0;
      return true;
    }
    return false;
  } else if constexpr (std::is_same_v<T, const char*>) {
    out = text;
    return true;
  } else if constexpr (std::is_integral_v<T>) {
    char* end = nullptr;
    const long long v = std::strtoll(text, &end, 0);
    if (end == text) return false;
    out = static_cast<T>(v);
    return true;
  } else if constexpr (std::is_floating_point_v<T>) {
    char* end = nullptr;
    const double v = std::strtod(text, &end);
    if (end == text) return false;
    out = static_cast<T>(v);
    return true;
  } else {
    static_assert(sizeof(T) == 0, "base::Option<T>: no string parser for this T");
    return false;
  }
}

}  // namespace detail

// Type-erased handle so every Option<T>, whatever T is, threads onto one
// self-registering list (the same InitChain trick base::Feature uses). Walk
// them with OptionBase::VisitAll, or let InitOptionsFromEnv() drive them.
class BASE_EXPORT OptionBase : public InitChain<OptionBase> {
 public:
  OptionBase(const char* opt_name, const char* env_var, const char* description)
      : name_(opt_name), env_(env_var), desc_(description) {
    Register(this);
  }

  OptionBase(const OptionBase&) = delete;
  OptionBase& operator=(const OptionBase&) = delete;
  virtual ~OptionBase() = default;

  // Parse `text` into the concrete value. On success marks the option
  // overridden and returns true; on a parse error leaves the value untouched
  // and returns false.
  bool SetFromString(const char* text) {
    if (!ParseFromString(text)) return false;
    overridden_ = true;
    return true;
  }

  // Put the option back on its compiled-in default.
  void Reset() {
    RestoreDefault();
    overridden_ = false;
  }

  // Write the current value as text a later SetFromString can parse back.
  // Returns the number of characters written, null terminator excluded.
  virtual mem_size FormatValue(char* buffer, mem_size buffer_size) const = 0;

  const char* name() const { return name_; }
  const char* env() const { return env_; }
  const char* desc() const { return desc_; }
  bool overridden() const { return overridden_; }

 protected:
  virtual bool ParseFromString(const char* text) = 0;
  virtual void RestoreDefault() = 0;

 private:
  const char* name_;
  const char* env_;
  const char* desc_;
  bool overridden_ = false;
};

// A named option. Read it through the implicit conversion or get():
//
//   namespace { base::Option<int> kWidth{"win.width", 1920, "REC_WIN_W"}; }
//   ...
//   desc.width = kWidth;            // uses the (possibly overridden) value
//
// Pass an env var name to make it overridable; leave it null for a pure
// programmatic default. Declare options at namespace scope, not as
// function-local statics: InitOptionsFromEnv() can only populate options that
// were already constructed (and thus registered) by the time it runs.
template <typename T>
class Option : public OptionBase {
 public:
  Option(const char* opt_name, T default_value, const char* env_var = nullptr,
         const char* description = "")
      : OptionBase(opt_name, env_var, description),
        value_(default_value),
        default_(default_value) {}

  operator const T&() const { return value_; }
  const T& get() const { return value_; }
  void set(T v) { value_ = v; }

  mem_size FormatValue(char* buffer, mem_size buffer_size) const override {
    if constexpr (std::is_same_v<T, const char*>) {
      if (!value_) {
        if (buffer_size) *buffer = '\0';
        return 0;
      }
    }
    return FormatTo(buffer, buffer_size, "{}", value_);
  }

 protected:
  bool ParseFromString(const char* text) override {
    return detail::ParseOption(text, value_);
  }

  void RestoreDefault() override { value_ = default_; }

 private:
  T value_;
  const T default_;
};

// Populate every registered option that names an environment variable from the
// current environment. Call once after static initialization (e.g. at startup,
// before any subsystem reads an option). Returns how many options an env value
// actually overrode.
inline mem_size InitOptionsFromEnv() {
  mem_size overridden = 0;
  OptionBase::VisitAll([&overridden](const OptionBase* registered) {
    // The chain stores const pointers; the options are mutable globals, so
    // writing through them here is well-defined.
    auto* option = const_cast<OptionBase*>(registered);
    const char* env = option->env();
    if (!env) return;
    if (const char* value = std::getenv(env))
      if (option->SetFromString(value)) ++overridden;
  });
  return overridden;
}

}  // namespace base
