// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "netbuffer.h"

namespace network {

#if 0
class NetAllocator final : public flatbuffers::Allocator {
 public:
  uint8_t* allocate(size_t size) override {
    throw std::logic_error("The method or operation is not implemented.");
  }

  void deallocate(uint8_t* p, size_t size) override {
    throw std::logic_error("The method or operation is not implemented.");
  }
};

NetBuffer::NetBuffer() : flatbuffers::FlatBufferBuilder() {}

NetBuffer::~NetBuffer() {}
#endif
}  // namespace network