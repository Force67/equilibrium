// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

class IDASyncClient;

class IDAEventHandler final {
 public:
  explicit IDAEventHandler(IDASyncClient&);
  ~IDAEventHandler();

 private:
  void HandleEvent(hook_type_t type, int code, va_list args);

  static ssize_t IdbEvent(void* userData, int code, va_list args);
  static ssize_t IdpEvent(void* userData, int code, va_list args);

 private:
  IDASyncClient& client_;
};