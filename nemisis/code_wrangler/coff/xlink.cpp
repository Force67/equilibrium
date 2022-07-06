
// Copyright (C) Force67 2019

#include "xlink.h"
#include "db/objdb.h"

#include "db/mapdb.h"

namespace link::coff
{
	extern Configuration* config;

	xLink::xLink()
	{
		// a new linker instance
		linkm = std::make_unique<Linker>();

		// the transformer analyzes, obfuscates, and emits new code
		codetf = std::make_unique<codeTransform>(database, linkm.get());
	}

	void xLink::inheritInformation()
	{
		auto& nt = database.getNt();
		config->subSystem = nt.optionalHeader.subsystem;
		
		if (database.isDll())
			config->target = PeTarget::TARGET_DLL;
	}

	bool xLink::loadInputs()
	{
		// load input file
		{
			utl::File inputFile(config->inputFile);
			if (!inputFile.IsOpen())
				return false;

			if (database.loadPe(inputFile) != ldbError::OK)
				return false;
		}

		// check if we find any map or pdb files
		{
			auto pdbFileName = database.getPdbDir();
			if (!pdbFileName.empty()) {
				utl::File pdbFile(pdbFileName);
				if (pdbFile.IsOpen()) {
					pdbInfo = std::make_unique<pdbObj>();
					pdbInfo->load(pdbFile);
				}
			}

			auto inputFileName = config->inputFile + "\\" + config->inputName + ".map";
			
			utl::File mapFile(inputFileName);
			if (mapFile.IsOpen()) {
				std::puts("Loaded map file");

				mapInfo = std::make_unique<mapDB>();
				mapInfo->load(mapFile);
			}
		}
		
	}

	bool xLink::process()
	{
		inheritInformation();

		// register default image sections
		linkm->createSections();

		if (codetf->init()) {

			for (auto& sec : database.sections) {
				auto& info = sec.header();

				// we only decompose code at that stage
				if (!(info.flags & pe::SEC_EXEC))
					continue;

				// disassemble using virtual address bases
				if (!codetf->disasmSection(sec))
					return false;
			}

			codetf->mergeBlueprints();
		}

		return true;
	}

	bool xLink::link()
	{
		utl::File outFile(config->outputFile, utl::fileMode::write);
		if (outFile.IsOpen()) {
			return linkm->EmitFile(outFile);
		}

		return false;
	}
}