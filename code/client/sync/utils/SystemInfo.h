// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
#pragma once

#include <qstring.h>

namespace noda::sync::utils
{
	const QString &GetSysUsername();
	const QString &GetHardwareId();
} // namespace noda::sync::utils