// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Storage.h"

namespace noda::sync::utils
{
  constexpr uint32_t kStorageVersion = 4;

  Storage::Storage() {}

  bool Storage::Initialize()
  {
	// TODO: migrate from old storage format
	bool result = _node.create("$ syncplugin_data");
	if(result) {
	  if(!_node.supset(DataIndex::NodaVersion, &kStorageVersion, sizeof(kStorageVersion)))
		return false;

	  return _node.supset(DataIndex::IdbVersion, 0u, sizeof(uint32_t));
	}

	// fetch current version
	uint32_t currentVersion = 0;
	_node.supval(DataIndex::NodaVersion, &currentVersion, sizeof(uint32_t));

	if(currentVersion < kStorageVersion) {
	  currentVersion = kStorageVersion;

	  if(!_node.supset(DataIndex::NodaVersion, &currentVersion, sizeof(uint32_t)))
		return false;
	}

	// Done
	return true;
  }

} // namespace noda::sync::utils