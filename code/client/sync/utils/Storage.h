// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include "IdaInc.h"
#include <netnode.hpp>

namespace noda
{
	namespace sync
	{
		namespace utils
		{
			class Storage
			{
			  public:
				Storage();

				bool Initialize();

			  private:
				enum DataIndex : nodeidx_t {
					NodaVersion,
					IdbVersion
				};

				netnode _node;
			};

		} // namespace utils
	}     // namespace sync
} // namespace noda
