// Copyright (C) 2021 Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "binding.h"
#include "tools/workbench.h"

// plain make a USI provided by given type name...
Binding<uint64_t, ea_t> MakeUSI("MakeUSI", [](ea_t ea) { 
	return work_bench()->ProduceUSI(ea);
});

// create an USI, directly export it to given database
// fails when executed on wrong thread.