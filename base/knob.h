#pragma once

#include <base/arch.h>
#include <base/memory/move.h>
#include <base/memory/cxx_lifetime.h>

namespace base {

  // Generic Prototype
  struct BasicKnob {};

  // Just a utility struct for the generated code.
  struct KnobEntry {
      const char* name;
      base::BasicKnob* knob_obj;
  };

  // Specialization for POD types
  template<typename T, bool is_trivial = base::IsTrivial<T>>
  struct Knob : public BasicKnob {
    static_assert(is_trivial, "This specialization is for POD types only.");

    T data{};

    // no-op
    void Construct() {}
    void Destruct() {}

    T& value() {
      return data;  // Direct access since it's POD
    }

    const T& value() const {
      return data;  // Direct access since it's POD
    }

    operator bool() { return data; }
  };

  // Specialization for non-POD types
  template <typename T>
  struct Knob<T, false> : public BasicKnob {
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