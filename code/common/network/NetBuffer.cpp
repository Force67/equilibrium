// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NetBuffer.h"

namespace network {

class NetAllocator final : public flatbuffers::Allocator {
  public:
  uint8_t *allocate(size_t size) override
  {
	throw std::logic_error("The method or operation is not implemented.");
  }

  void deallocate(uint8_t *p, size_t size) override
  {
	throw std::logic_error("The method or operation is not implemented.");
  }
};


NetBuffer::NetBuffer() : 
	flatbuffers::FlatBufferBuilder()
{

}

NetBuffer::~NetBuffer()
{

}
}