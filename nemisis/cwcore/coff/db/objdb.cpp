
// Copyright (C) Force67 2019

#include "objdb.h"

#include <fmtdef.h>
#include <utl/File.h>

namespace link::coff
{
	uint16_t cvtPeMachine();

	coffObj::coffObj(const char* name) :
		objName(name)
	{}

	coffObjErr coffObj::load(utl::File& file)
	{
		//file.Seek(0, utl::seekMode::seek_set);
		file.Read(header);

		// non matching architecture
		if (header.machine != cvtPeMachine())
			return coffObjErr::badmagic;

		for (int i = 0; i < header.numSections; i++) {
			pe::sectionHeader secHeader{};

			if (!file.Read(secHeader))
				return coffObjErr::badsec;

			// skip it if no data exists
			if (secHeader.pointerToRawData == 0)
				continue;

			auto ofs = file.Tell();

			// copy data
			file.Seek(secHeader.pointerToRawData, utl::seekMode::seek_set);

			auto& chunk = sections.emplace_back(this, &secHeader);
			file.Read(chunk.data);

			// and back to the next header
			file.Seek(ofs, utl::seekMode::seek_set);
		}

		for (auto& chunk : sections) {
			// copy possible relocations
			if (chunk.header.pointerToRelocations) {
				file.Seek(chunk.header.pointerToRelocations, utl::seekMode::seek_set);

				for (uint16_t r = 0; r < chunk.header.numberOfRelocations; r++) {
					pe::CoffReloc item{};
					file.Read(item);

					chunk.relocs.push_back(item);
				}
			}
		}

		// load string table
		uint64_t num = static_cast<uint64_t>(header.numSymbols) * 18;
		file.Seek(num + header.pointerSymbolTable, utl::seekMode::seek_set);

		// number of bytes used
		uint32_t numbytes = 0;
		file.Read(numbytes);

		// and read the table into memory
		strTab.resize(numbytes - sizeof(uint32_t));
		file.Read(strTab);

		file.Seek(header.pointerSymbolTable, utl::seekMode::seek_set);
		for (uint32_t i = 0; i < header.numSymbols; i++) {
			pe::CoffSym sym{};
			file.Read(sym);

			// we hit a function
			if (sym.complexType == 32) {

				if (sym.nameZeroes == 0 && sym.dontAnnoyMe != 0) {
					uint32_t ofsStrTab = sym.dontAnnoyMe;
					const char* str = reinterpret_cast<const char*>(&strTab[ofsStrTab - 4]);
				
					int16_t realIdx = sym.secNum - 1;
					for (size_t x = 0; x < sections.size(); x++) {
						if (x == realIdx) {
							funcRecord fr{};
							fr.data = &sections[x];
							fr.name = str;

							funcs.emplace_back(fr);
							break;
						}
					}
				}
			}

			for (uint8_t a = 0; a < sym.numAuxSymbols; a++) {
				file.Seek(18, utl::seekMode::seek_cur);
				symbols.emplace_back(sym);
				++i;
			}

			symbols.emplace_back(sym);
		}

		return coffObjErr::none;
	}

	SectionChunk* coffObj::getObject(const char* target)
	{
		for (auto& sym : symbols) {
			const char* name = nullptr;

			// long name
			if (sym.nameZeroes == 0 && sym.dontAnnoyMe != 0) {
				uint32_t ofs = sym.dontAnnoyMe;

				// subtract by size of the first 4 bytes which we don't store
				name = reinterpret_cast<const char*>(&strTab[ofs - 4]);
			}
			else {
				name = sym.name;
			}

			// not found
			if (std::strncmp(name, target, 8) != 0)
				continue;
			
			int16_t realIdx = sym.secNum - 1;
		
			for (size_t x = 0; x < sections.size(); x++) {
				if (x == realIdx) {
					return &sections[x];
				}
			}
		}

		return nullptr;
	}

	SectionChunk* coffObj::getFunc(const char* name)
	{
		for (auto& it : funcs) {
			if (std::strcmp(it.name, name) == 0)
				return it.data;
		}

		return nullptr;
	}
}