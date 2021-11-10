// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "script_binding.h"
#include "tools/workbench.h"

#if 0
// plain make a USI provided by given type name...
Binding<uint64_t, ea_t> MakeUSI("MakeUSI", [](ea_t ea) { 
	return work_bench()->ProduceUSI(ea);
});
#endif
// create an USI, directly export it to given database
// fails when executed on wrong thread.