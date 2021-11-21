// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "application.h"

// uncomment the two lines below to enable correct color spaces
//#define GL_FRAMEBUFFER_SRGB 0x8DB9
//#define GL_SRGB8_ALPHA8 0x8C43

// https://gist.github.com/ad8e/dd150b775ae6aa4d5cf1a092e4713add

int main(void) {
  Application app;
  return app.Exec();
}