#pragma once

// Copyright (C) Force67 2019

#include <cstdint>
#include <string.h>
#include <vector>
#include <fmtdef.h>
#include <llvm/mathext.h>

namespace utl {
	class File;
}

// heavily based on llvm LLD
namespace link::coff
{
	class coffObj;

	// a chunk represents a bit of data, that is to be placed into output section later
	class Chunk {
	public:

		enum class Kind : uint8_t { SectionKind, OtherKind, ImportThunkKind };
		Kind kind() const { return chunkKind; }

		Chunk(Kind k = Kind::OtherKind) : chunkKind(k) {}

		bool backingData() {
			return !data.empty();
		}

		uint32_t getAlignment() const { return 1U << p2Align; }
		void setAlignment(uint32_t align) {
			// Treat zero byte alignment as 1 byte alignment.
			align = align ? align : 1;
			p2Align = llvm::Log2_32(align);
		}

		uint32_t getRVA() const { return rva; }
		void setRVA(uint64_t v) {
			rva = (uint32_t)v;
		}

		size_t getSize() {
			return data.size();
		}

		// write directly to outFile
		virtual void writeTo(utl::File&) {};

	public:
		std::vector<uint8_t> data;
		bool useFatPadding{ true };

	protected:

		uint8_t p2Align : 7;
		Kind chunkKind;
		uint32_t rva = 0;
	};

	class NonSectionChunk : public Chunk {
	public:
		virtual ~NonSectionChunk() = default;

		// Returns the size of this chunk (even if this is a common or BSS.)
		virtual size_t getSize() const = 0;
		virtual uint32_t getOutputCharacteristics() const { return 0; }
		static bool classof(const Chunk * c) { return c->kind() != Kind::SectionKind; }

	protected:
		NonSectionChunk(Kind k = Kind::OtherKind) : Chunk(k) {}
	};

	// represents a section chunk, based off coff data
	class SectionChunk : public Chunk {
	public:
		SectionChunk(coffObj *obj, pe::sectionHeader*);
		Kind kind() const { return Kind::SectionKind; }

		void writeTo(utl::File&) override;

	private:

		friend class coffObj;

		void applyRelX64(uint8_t* off, uint32_t s, uint64_t p, uint16_t t);

		coffObj* obj;
		std::vector<pe::CoffReloc> relocs{};

	public:

		pe::sectionHeader header{};
	};

	class Section {
	public:

		Section(const char* name, uint32_t chars) : 
			chunksneedAssembly(true) {
			strcpy(secHeader.name, name);
			secHeader.flags = chars;
		}

		Section(const pe::sectionHeader& from) :
			chunksneedAssembly(false) {
			std::memcpy(&secHeader, &from, sizeof(pe::sectionHeader));
		}

		Section() = default;

		void merge(Section*);
		void addChunk(Chunk*);
		void writeHeader(uint8_t*);

		inline bool hasChunks() {
			return !chunks.empty();
		}

		inline uint32_t virtualAddress() const {
			return secHeader.virtualAddress;
		}

		inline uint32_t virtualSize() const {
			return secHeader.misc.vs;
		}

		inline uint32_t ptrRawData() const {
			return secHeader.pointerToRawData;
		}

		inline uint32_t rawSize() const {
			return secHeader.sizeOfRawData;
		}

		inline pe::sectionHeader& header() {
			return secHeader;
		}

		// it is illegal to call this with an unassembled chunk
		inline uint8_t* data() const {
			if (chunks.size() == 1 && !chunksneedAssembly)
				return chunks.at(0)->data.data();

			return nullptr;
		}

	private:

		pe::sectionHeader secHeader{};

	public:

		std::vector<pe::CoffReloc*> relocs;
		bool chunksneedAssembly;
		std::vector<Chunk*> chunks;
	};
}