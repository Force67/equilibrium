// Copyright (C) 2024 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
//
// Feature flag implementation.

#pragma once

#include <base/arch.h>
#include <base/memory/move.h>
#include <new>

namespace base {

// Just a utility struct for the generated code.
struct KnobEntry {
    const char* name;
    base::BasicKnob* knob_obj;
};

struct BasicKnob {};

// Usage: global Knob<bool> MyBooleanOption;
// Then run python /equilibrium/base/gen_knobs.py . knobs.h
// It will generate:
// - void InitializeAllKnobs()
// - void DestructAllKnobs()
// and void InitializeAllKnobsAndRegister(KnobEntry (&knob_list)[kKnobCount])
template <typename T>
struct Knob final : public BasicKnob {
  using Type = T;

  template <typename... Args>
  void Construct(Args&&... args) {
    new (&data_) T(base::forward<Args>(args)...);
  }

  void Destruct() {
    reinterpret_cast<T*>(&data_)->~T();
  }

  T& value() {
    return *reinterpret_cast<T*>(&data_);
  }

  const T& value() const {
    return *reinterpret_cast<const T*>(&data_);
  }

 private:
  alignas(T) byte data_[sizeof(T)]{};
};

}  // namespace base