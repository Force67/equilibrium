
// Copyright (C) Force67 2019

#include "linkdriver.h"
#include "linker.h"
#include "xlink.h"

namespace link::coff
{
	static Configuration g_config{};
	Configuration* config = &g_config;

	static uint64_t getDefaultImageBase() {
		if (config->machine == cwArchitecture::AMD64)
			return config->target == PeTarget::TARGET_DLL ? 0x180000000 : 0x140000000;
		return config->target == PeTarget::TARGET_DLL ? 0x10000000 : 0x400000;
	}

	static void setupCommonOpts() {
		if (config->buildTime == UINT32_MAX) {
			config->buildTime = _time32(nullptr);
		}
	}

	bool linkPe(const std::vector<std::string>& args, utl::File &out)
	{
		setupCommonOpts();

		// if no /base was specified
		if (config->imageBase == uint64_t(-1)) {
			config->imageBase = getDefaultImageBase();
		}

		auto linkOps = std::make_unique<Linker>();
		if (!linkOps->EmitFile(out)) {
			std::puts("[linkPe]: Unable to emit file");
			return false;
		}

		return true;
	}

	bool relinkPe()
	{
		setupCommonOpts();

		config->imageBase = getDefaultImageBase();

		auto linkOps = std::make_unique<xLink>();
		if (!linkOps->loadInputs()) {
			std::puts("unable to load input files");
			return false;
		}

		if (!linkOps->process()) {
			std::puts("unable to process");
			return false;
		}

		return linkOps->link();
	}
}