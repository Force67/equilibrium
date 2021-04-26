// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

namespace tools {
class Signature {
 public:
  Signature(){};

  std::string ToSmallString();
  std::string ToIdaString();

 private:
  qvector<uchar> bytes_;
  qvector<uchar> masks_;
};
}
