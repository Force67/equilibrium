
// Copyright (C) Force67 2019

#include "ldb.h"
#include "../linkdriver.h"

namespace link::coff
{
	extern Configuration* config;

	// from database on disk
	bool ldb::deserialize(utl::File& in)
	{
		static constexpr uint16_t ldbMagic = 0xD8D8;

		uint16_t magic = 0;
		in.Read(magic);

		if (magic != ldbMagic)
			return false;

		//TODO:::

		return false;
	}

	ldbError ldb::loadPe(utl::File& file)
	{
		// ensure we start from scratch
		file.Seek(0, utl::seekMode::seek_set);
		file.Read(dosHdr);

		// validate header again
		if (dosHdr.magic != pe::dosMagic)
			return ldbError::BADMAGIC;

		file.Seek(dosHdr.lfanew, utl::seekMode::seek_set);
		file.Read(ntHdr);

		// next, load the segments
		for (int i = 0; i < ntHdr.fileHeader.numSections; i++) {
			pe::sectionHeader secHeader{};
			if (file.Read(secHeader)) {

				// skip it if no data exists
				if (secHeader.pointerToRawData == 0)
					continue;

				auto ofs = file.Tell();

				// copy data
				file.Seek(secHeader.pointerToRawData, utl::seekMode::seek_set);

				auto* chunk = new Chunk;
				file.Read(chunk->data, secHeader.sizeOfRawData);

				// create a new section entry
				auto& sec = sections.emplace_back(secHeader);
				sec.addChunk(chunk);

				file.Seek(ofs, utl::seekMode::seek_set);
			}
			else {
				return ldbError::SEGERR;
			}
		}

		// quick hacky way of getting pdb dir
		auto* dDir = &ntHdr.optionalHeader.directories[pe::imageDirectoryEntry::DEBUG_DIR];
		if (dDir->va) {
			auto* dbgDir = makeOffset<pe::debugDirectory>(dDir->va);
		
			for (int i = 0; i < dDir->size / sizeof(pe::debugDirectory); i++) {
				if (dbgDir->type == pe::debugType::CODEVIEW) {

					if (!dbgDir->sizeData)
						continue;

					auto* strPtr = getDataPtr(dbgDir->adressRawData) + sizeof(pe::debugCodeView_v70);
					const size_t pathLen = dbgDir->sizeData - sizeof(pe::debugCodeView_v70);

					pdbDir.resize(pathLen);
					std::memcpy(pdbDir.data(), strPtr, pathLen);

					break;
				}
				dbgDir++;
			}
		}

		return ldbError::OK;
	}

	uint8_t* ldb::getDataPtr(uint32_t rva)
	{
		for (auto& sec : sections) {
			if (rva >= sec.virtualAddress() && rva < sec.virtualAddress() + sec.virtualSize()) {
				return sec.data() + (rva - sec.virtualAddress()/* + sec.ptrRawData() removed, cause we are within our section bit*/);
			}
		}

		return nullptr;
	}

	uint32_t ldb::getOfs(uint32_t rva)
	{
		for (auto& sec : sections) {
			if (rva >= sec.virtualAddress() && rva < sec.virtualAddress() + sec.virtualSize()) {
				return rva - sec.virtualAddress();
			}
		}

		return 0;
	}

	uint8_t* ldb::getVirtualPtr(uint32_t ofs)
	{
		for (auto& sec : sections) {
			if (ofs >= sec.ptrRawData() && ofs <= sec.ptrRawData() + sec.rawSize()) {
				// this is wrong
				//return ofs + sec.virtualAddress() - sec.ptrRawData();
			}
		}

		return nullptr;
	}

	uint32_t ldb::getExportRva(const char* name)
	{
		auto& exp = ntHdr.optionalHeader.directories[pe::EXPORT_DIR];
		if (exp.size == 0)
			return 0;

		auto desc = makeOffset<pe::exportDirectory>(exp.va);
		if (desc) {

			auto* funcs = makeOffset<uint32_t>(desc->addressOfFunctions);
			auto* ordinals = makeOffset<uint16_t>(desc->addressOfNameOrdinals);
			auto* names = makeOffset<uint32_t>(desc->addressOfNames);

			for (uint32_t i = 0; i < desc->numFunctions; i++) {

				// ensure that we don't hit an ordinal
				if ((!(funcs[i] & 0x80000000)) /*32 bit snap*/ &&
					(!(funcs[i] & 0x8000000000000000 /*64 bit snap*/))) {

					const auto* ename = makeOffset<const char>(names[i]);
					if (ename) {
						if (std::strcmp(name, ename) == 0) {
							return funcs[i];
						}
					}
				}
			}
		}

		return 0;
	}
}