// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include <memory>
#include "FileTransfer.h"

namespace noda::net {
  FileTransfer::FileTransfer()
  {
  }

  bool FileTransfer::SendFile(const qstring &filePath)
  {
	if(FILE *fp = qfopen(filePath.c_str(), "rb")) {
	  qfseek(fp, 0, SEEK_SET);
	  size_t sz = qftell(fp);

	  fileBuf = std::make_unique<uint8_t[]>(sz);
	  qfread(fp, fileBuf.get(), sz);

	  return true;
	}

	return false;
  }
} // namespace noda::net