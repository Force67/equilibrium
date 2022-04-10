// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "ui/element.h"
#include <functional>

namespace ui {

template <typename TState>
struct StatefulElement : public Element {
 private:
  TState _state = {};

 public:
  std::function<void(const TState&)> on_state_change;

  void state(TState state) {
    _state = state;

    if (on_state_change) {
      on_state_change(_state);
    }

    // should_relayout();
    // should_repaint();
  }

  const TState& state() const { return _state; }
};
}  // namespace ui