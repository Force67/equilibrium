// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Storage.h"
#include "utils/Logger.h"

namespace noda::sync::utils {
  constexpr uint32_t kStorageVersion = 4;
  constexpr char kStorageName[] = "$ noda_data";

  Storage::Storage() {}

  bool Storage::Initialize()
  {
	uint32_t oldVersion = -1;

	// delete old sync plugin data
	netnode oldNode;
	if(netnode_check(&oldNode, "$ syncplugin_data", 0, false)) {
	  oldNode.supval(0u, &oldVersion, sizeof(uint32_t));
	  oldNode.kill();
	}

	bool result = _node.create(kStorageName);
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

	if(oldVersion != -1) {
	  LOG_INFO("Migrating from legacy sync plugin version {} to version {}",
	           oldVersion, currentVersion);
	}

	// Done
	return true;
  }

} // namespace noda::sync::utils