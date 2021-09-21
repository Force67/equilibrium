// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// a set of wrappers around the often annoying ida api.
#pragma once

#include "ial/idb.h"
#include "ial/event.h"

#include <QString>
#include <functional>

namespace utils {
template <typename T>
struct RequestFunctor final : exec_request_t {
 public:
  RequestFunctor(std::function<T> callback, int flags) {
    callback_ = callback;
    execute_sync(*this, flags);
  }

  int execute() override {
    callback_();
    return 0;
  }

 private:
  std::function<T> callback_;
};
}  // namespace utils