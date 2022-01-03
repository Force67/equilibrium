
// Copyright (C) Force67 2019

#include "transform.h"
#include "linkdriver.h"
#include "linker.h"

namespace link::coff
{
	extern Configuration* config;
	cwDebugInformation g_dbgInfo{};

	codeTransform::codeTransform(ldb &db, Linker *link) :
		db(db),
		link(link)
	{}

	bool codeTransform::init()
	{
		// check if image was linked against SDK
		spotmarkerAddr = db.getExportRva("cw_spotcheck_marker");
		triggermarkerAddr = db.getExportRva("cw_trigger_marker");
		if (spotmarkerAddr && triggermarkerAddr) {
			std::puts("[ct] Found SDK features");
		}

		// attempt to load blueprints
		if (config->enableDiscProtect) {
			loadBlueprint("aes.obj");
		}

		return disasm::init();
	}

	// a blueprint consists of coff code objects which we
	// use to stitch together new sections
	bool codeTransform::loadBlueprint(const char* name)
	{
		auto& dbx = blueprints.emplace_back(name);
		
		utl::File file(name);
		if (file.IsOpen()) {
			auto state = dbx.load(file);

			const char* errStr = nullptr;
			switch (state) {
			case coffObjErr::badmagic: {
				errStr = "[coffObj] bad magic"; break;
			}
			case coffObjErr::badsec: {
				errStr = "[coffObj] bad section"; break;
			}
			case coffObjErr::none:
			default:
				g_dbgInfo.numBlueprintsUsed++;
				std::printf("[ct] loaded blueprint %s\n", name);
				return true;
			}

			std::puts(errStr);
		}

		return false;
	}

	// merges the blueprints into linkable output sections
	void codeTransform::mergeBlueprints()
	{
		auto findBp = [&](const char* name) -> coffObj*
		{
			for (auto& it : blueprints) {
				if (it.name() == name)
					return &it;
			}

			return nullptr;
		};

		auto* sec = link->getSection(".cwsc");
		if (sec) {

			auto* obj = findBp("aes.obj");
			if (obj) {
				for (auto& chunk : obj->getSections()) {
					// code only
					if (chunk.header.flags & pe::SEC_CODE) {
						chunk.setAlignment(8);
						sec->addChunk(&chunk);
					}
				}
			}
		}
	}

	bool codeTransform::disasmSection(Section& sec)
	{
		bool res = disasm::process(sec.data(), sec.rawSize(), sec.virtualAddress());
		if (res) {
			switch (config->machine) {
			case cwArchitecture::AMD64:
				return Diasm_AMD64(sec.data());
			case cwArchitecture::INTEL86:
				return Diasm_X86();
			}
		}

		return false;
	}

	// for debugging
	static void printOpInfo(const cs_x86_op& op) {
		std::printf("Operand: Type %d, Reg %d, (Mem: base %d)\n", op.type, op.reg, op.mem.base);
	}

	bool codeTransform::Diasm_AMD64(const uint8_t *base)
	{
		for (size_t bi = 0; bi < diCount; bi++) {
			cs_insn* ins = &(insn[bi]);

			if (!ins->detail) {
				std::puts("Warning: No instruction details available.. continuing");
				continue;
			}

			auto detail = ins->detail->x86;
			uint32_t dest = static_cast<uint32_t>(X86_REL_ADDR(*ins));

			switch (ins->id) {
			case X86_INS_LEA: {
				//auto base = detail.operands[0];
				auto op = detail.operands[1];

				//printOpInfo(base);
				//printOpInfo(op);

				// string access
				if (op.type == X86_OP_MEM && op.mem.base == X86_REG_RIP) {

					if (curMarker && !markerInfofetched) {
						// store description for later use
						curMarker->strDesc = reinterpret_cast<const char*>(db.getDataPtr(dest));
						markerInfofetched = true;
					}
				}
			}
			case X86_INS_CALL: {
				auto op = detail.operands[0];

				// regular call
				if (op.size == 8 && op.type == X86_OP_IMM) {

					// collect spot markers
					if (dest == spotmarkerAddr || dest == triggermarkerAddr) {
						if (codemarkerStart) {

							if (!curMarker) {
								std::puts("[Diasm] SDK marker logic bug encountered (lea should have followed immediately?!?)");
								__debugbreak();
							}

							curMarker->markerAddr = codemarkerStart;
							curMarker->markerSize = static_cast<uint32_t>(ins->address - codemarkerStart);
							curMarker = nullptr;
						}
						else {
							codemarkerStart = static_cast<uint32_t>(ins->address);
							markerInfofetched = false;

							curMarker = &codeMarkers.emplace_back();
							curMarker->markerType = dest == spotmarkerAddr 
								? codeMarkerType::MARKER_SPOT 
								: codeMarkerType::MARKER_TRIGGER;
						}
					}
				}
			}
			}
		}

		return true;
	}

	bool codeTransform::Diasm_X86()
	{
		return false;
	}
}