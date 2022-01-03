#pragma once

// Copyright (C) Force67 2019

#include <fmtdef.h>
#include <string>
#include "../chunk.h"

namespace utl {
	class File;
}

namespace link::coff
{
	struct symRecord
	{
		std::string symName;
		uintptr_t symRva;
	};

	class mapDB
	{
	public:

		bool load(utl::File&);

	private:
		std::string targetName;
		uintptr_t loadAddress;
		std::vector<symRecord> symr;
	};
}