// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <memory>
#include "IdaInc.h"

namespace noda {
  class FileTransfer {
  public:
	FileTransfer();

	bool SendFile(const qstring &);

  private:
	std::unique_ptr<uint8_t[]> fileBuf;
  };
} // namespace noda