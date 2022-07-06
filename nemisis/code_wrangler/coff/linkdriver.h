#pragma once

// Copyright (C) Force67 2019

#include <vector>
#include <string>
#include <fmtdef.h>
#include <ctime>

#include "cwcore.h"

namespace link::coff
{
	enum class PeTarget
	{
		TARGET_EXE,
		TARGET_DLL,
		TARGET_COFFOBJ
	};

	struct Configuration
	{
		// target opt
		cwArchitecture machine{ cwArchitecture::AMD64 };
		PeTarget target{ PeTarget::TARGET_EXE };
		uint16_t subSystem{ pe::subsystemType::SYS_GUI };
		uint64_t imageBase{ UINT64_MAX };
		uint32_t buildTime{ UINT32_MAX };
		uint32_t functionPadMin = 10;
		uint64_t fileAlignment{ 512 };
		uint64_t align{ 4096 };
		uint64_t stackReserve = 1024 * 1024;
		uint64_t stackCommit = 4096;
		uint64_t heapReserve = 1024 * 1024;
		uint64_t heapCommit = 4096;

		// link opt
		bool skipDosStub = false;
		bool skipCWInfo = false;

		// transform opt
		bool enableDiscProtect = true;
		bool enableScrambleRtti = true;

		// path
		std::string inputFile;
		std::string inputName;
		std::string outputFile;
		std::string rootDir;

		bool is64() { return machine == cwArchitecture::AMD64 || machine == cwArchitecture::ARM64; }
	};

	// regular link operation with arguments from command line
	bool linkPe(const std::vector<std::string> &args, utl::File& out);

	// unlinks and then relinks exiting image
	bool relinkPe();
}