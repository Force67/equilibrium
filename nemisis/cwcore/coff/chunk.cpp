
// Copyright (C) Force67 2019

#include "chunk.h"
#include "linkdriver.h"
#include "db/objdb.h"
#include <assert.h>
#include <llvm/endian.h>

namespace le = llvm::support::endian;

static void add16(uint8_t* p, int16_t v) { le::write16le(p, le::read16le(p) + v); }
static void add32(uint8_t* p, int32_t v) { le::write32le(p, le::read32le(p) + v); }
static void add64(uint8_t* p, int64_t v) { le::write64le(p, le::read64le(p) + v); }
static void or16(uint8_t* p, uint16_t v) { le::write16le(p, le::read16le(p) | v); }
static void or32(uint8_t* p, uint32_t v) { le::write32le(p, le::read32le(p) | v); }

namespace link::coff
{
	extern Configuration* config;

	void Section::merge(Section* other) {
		chunks.insert(chunks.end(), other->chunks.begin(), other->chunks.end());
		other->chunks.clear();
	}

	void Section::addChunk(Chunk* c) {
		chunks.push_back(c);
	}

	void Section::writeHeader(uint8_t* data) {
		std::memcpy(data, &secHeader, sizeof(pe::sectionHeader));
	}

	SectionChunk::SectionChunk(coffObj* parent, pe::sectionHeader *sec) :
		obj(parent)
	{
		data.resize(sec->sizeOfRawData);
		std::memcpy(&header, sec, sizeof(pe::sectionHeader));
	}

	void SectionChunk::writeTo(utl::File& out)
	{
		auto& symbols = obj->getSymbols();
		auto& chunks = obj->getSections();

		for (pe::CoffReloc &rel : relocs) {
			assert(rel.symTabIdx < symbols.size());

			auto& sym = symbols[rel.symTabIdx];

			if (sym.secNum == -1) {
				std::puts("FIXME: absolute sec num " __FUNCTION__);
			}

			// ignore undefined, absolute and debug
			if (sym.secNum == 0 || sym.secNum == -1 || sym.secNum == -2)
				continue;

			auto& chunk = chunks[sym.secNum - 1];

			uint8_t* off = data.data() + rel.vAddr;
			uint32_t s = chunk.getRVA() + sym.value;
			uint64_t p = static_cast<uint64_t>(rva) + rel.vAddr;

			switch (config->machine) {
			case cwArchitecture::AMD64: {
				applyRelX64(off, s, p,rel.type);
				break;
			}
			default: {
				std::puts("FIXME: unknown machine type in " __FUNCTION__);
			}
			}
		}
	}

	void SectionChunk::applyRelX64(uint8_t* off, uint32_t s, uint64_t p, uint16_t type)
	{
		switch (type) {
		case pe::imageRelocations::REL_AMD64_ADDR32: 
			add32(off, s + config->imageBase); break;
		case pe::imageRelocations::REL_AMD64_ADDR64:
			add64(off, s + config->imageBase); break;
		case pe::imageRelocations::REL_AMD64_ADDR32NB:
			add32(off, s); break;
		case pe::imageRelocations::REL_AMD64_REL32:
			add32(off, s - p - 4); break;
		case pe::imageRelocations::REL_AMD64_REL32_1:
			add32(off, s - p - 5); break;
		case pe::imageRelocations::REL_AMD64_REL32_2:
			add32(off, s - p - 6); break;
		case pe::imageRelocations::REL_AMD64_REL32_3:
			add32(off, s - p - 7); break;
		case pe::imageRelocations::REL_AMD64_REL32_4:
			add32(off, s - p - 8); break;
		case pe::imageRelocations::REL_AMD64_REL32_5:
			add32(off, s - p - 9); break;
		default:
			std::printf("unhandeled rel! %d(%x)\n", type, type); break;
		//case pe::imageRelocations::REL_AMD64_SECREL:

		}
	}
}