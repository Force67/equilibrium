
// Copyright (C) Force67 2019

#include "disasm.h"
#include "linkdriver.h"

namespace link::coff
{
	extern Configuration* config;

	disasm::~disasm() {
		shutdown();
	}

	bool disasm::init()
	{
		cs_arch arch;

		// convert machine
		switch (config->machine) {
		case cwArchitecture::AMD64:
		case cwArchitecture::INTEL86:
			arch = CS_ARCH_X86; break;
		case cwArchitecture::ARM:
			arch = CS_ARCH_ARM; break;
		case cwArchitecture::ARM64:
			arch = CS_ARCH_ARM64; break;
		default:
			arch = CS_ARCH_ALL; break;
		}

		auto err = cs_open(arch, CS_MODE_64, &handle);

		// todo: forward err to log handler
		const char* errStr = "Unknown";
		switch (err) {
		case CS_ERR_ARCH:
			errStr = "[disasm] Bad architecture"; break;
		case CS_ERR_MEM:
			errStr = "[disasm] Not enough memory for decomposing image"; break;
		case CS_ERR_OK: {

			// setup disasm options...
			cs_option(handle, CS_OPT_DETAIL, CS_OPT_ON);
			return true;
		}
		default:
			return false;
		}

		std::puts(errStr);
		return false;
	}

	void disasm::shutdown()
	{
		if (handle) {
			cs_free(insn, diCount);
			cs_close(&handle);

			// just to be sure...
			handle = 0;
		}
	}

	bool disasm::process(const uint8_t* data, size_t len, uint64_t virtAddrBase, size_t subset /* = 0 */)
	{
		diCount = cs_disasm(handle, data, len, virtAddrBase, subset, &insn);
		if (diCount > 0) {
			return true;
		}

		std::puts("[disasm] Failed to disassemble given code!");
		return false;
	}
}