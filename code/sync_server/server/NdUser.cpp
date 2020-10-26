// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "NdUser.h"

namespace noda {

  NdUser::NdUser(uint32_t id, std::string name, std::string guid) :
      _netId(id)
  {
	_name = std::move(name);
	_guid = std::move(guid);
  }

} // namespace noda