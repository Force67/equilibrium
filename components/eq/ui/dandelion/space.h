// Copyright (C) 2022 Vincent Hengel.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

//#include <ui/keycode.inl>

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

  float width() const { return geom_.width; }
  float height() const { return geom_.height; }

  void SetSize(float w, float h);
 private:
  Space(const Space&) = delete;
  Space& operator=(const Space&) = delete;

  virtual bool onAnimate(double /*nanos*/) { return false; }
  virtual void onOnceBeforeDraw() {}

 private:
  Geometry geom_;
};
}