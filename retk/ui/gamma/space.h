// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <ui/keycode.inl>
#include <core/SkTypes.h>

class SkCanvas;

namespace user_interface {

class Space {
 public:
  Space();
  ~Space();

  struct Geometry {
    float width = 0.f;
    float height = 0.f;
  };

  virtual void onDraw(SkCanvas*){};
  virtual bool onChar(SkUnichar) { return false; }

  float width() const { return geom_.width; }
  float height() const { return geom_.height; }

  void SetSize(float w, float h);
 private:
  Space(const Space&) = delete;
  Space& operator=(const Space&) = delete;

  virtual void OnResize();

  virtual void onDrawBackground(SkCanvas*);
  virtual void onDrawContent(SkCanvas*) = 0;
  virtual bool onAnimate(double /*nanos*/) { return false; }
  virtual void onOnceBeforeDraw() {}

 private:
  Geometry geom_;
};
}