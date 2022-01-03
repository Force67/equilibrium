#pragma once

// Copyright (C) Force67 2019

#include "linkdriver.h"
#include "chunk.h"
#include <utl/File.h>
#include <llvm/mathext.h>

namespace link::coff
{
	class outputSection
	{
	public:

		explicit outputSection(const char*);

		pe::sectionHeader& header() {
			return secHeader;
		}

		void addChunk(Chunk*);
		uint64_t getRVA() { return secHeader.virtualAddress; }

	private:
		std::string secName;
		pe::sectionHeader secHeader{};
	public:
		std::vector<Chunk*> chunks;
	};

	class Linker
	{
	public:
		// write newely linked file to disk
		bool EmitFile(utl::File&);

	private:

		// grand access to cwd components
		friend class codeTransform;
		friend class xLink;

		outputSection* getSection(const char* name);

		// the image headers, this is the _final_ step
		template<typename THeader> void EmitHeader(utl::File&);

		void createSections();
		void removeUnusedSections();
		void assignAddresses();
		void writeSections(utl::File&);

		uint64_t sizeHeaders = 0;
		uint64_t fileSize = 0;
		uint64_t sizeOfImage = 0;
		
		std::vector<outputSection> outSecs;
		outputSection* cwscSec{ nullptr };
		outputSection* textSec{ nullptr };
		outputSection* rdataSec{ nullptr };
		outputSection* buildidSec{ nullptr };
		outputSection* dataSec{ nullptr };
		outputSection* pdataSec{ nullptr };
		outputSection* idataSec{ nullptr };
		outputSection* edataSec{ nullptr };
		outputSection* didatSec{ nullptr };
		outputSection* rsrcSec{ nullptr };
		outputSection* relocSec{ nullptr };
		outputSection* ctorsSec{ nullptr };
		outputSection* dtorsSec{ nullptr };
	};
}