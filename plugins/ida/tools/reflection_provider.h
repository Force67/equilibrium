// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <utils/ial/event.h>
#include <tilted_reflection/symbol_database.h>

class ReflectionProvider final : public ial::UiEventHandler {
 public:
  ReflectionProvider(const char* path);

  bool AddSymbol(const ea_t);

 private:
  void HandleEvent(ui_notification_t code, va_list args) override;

 private:
  tilted_reflection::SymbolDatabase database_;
};