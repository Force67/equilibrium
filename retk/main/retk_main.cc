// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
// Global RETK app entry point.

#include "application.h"

int ReTKMain(int argc, const char** argv) {
  std::unique_ptr<Application> app_instance{std::make_unique<Application>()};

  return app_instance->Exec();
}