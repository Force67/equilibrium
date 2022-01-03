#pragma once

// Copyright (C) Force67 2019

#include <capstone/capstone/capstone.h>

namespace link::coff
{
	// quick wrapper around capstone
	class disasm
	{
	public:

		~disasm();
		bool init();
		void shutdown();
		bool process(const uint8_t*, size_t size, uint64_t virtAddrBase, size_t subset = 0);

	protected:

		size_t diCount = 0;
		csh handle = 0;
		cs_insn* insn = nullptr;
	};
}