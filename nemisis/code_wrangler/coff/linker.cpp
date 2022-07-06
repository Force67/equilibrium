
// Copyright (C) Force67 2019

#include "linker.h"
#include <llvm/mathext.h>

namespace link::coff
{
	extern Configuration* config;
	extern cwDebugInformation g_dbgInfo;

	// "This program cannot be run in DOS mode"
	static const uint8_t dosProgram[] = {
		0x0e, 0x1f, 0xba, 0x0e, 0x00, 0xb4, 0x09, 0xcd, 0x21, 0xb8, 0x01, 0x4c,
		0xcd, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6f, 0x67, 0x72,
		0x61, 0x6d, 0x20, 0x63, 0x61, 0x6e, 0x6e, 0x6f, 0x74, 0x20, 0x62, 0x65,
		0x20, 0x72, 0x75, 0x6e, 0x20, 0x69, 0x6e, 0x20, 0x44, 0x4f, 0x53, 0x20,
		0x6d, 0x6f, 0x64, 0x65, 0x2e, 0x24, 0x00, 0x00
	};

	static constexpr int dosStubSize = sizeof(pe::dosHeader) + sizeof(dosProgram);
	static_assert(dosStubSize % 8 == 0, "DOSStub size must be multiple of 8");

	uint16_t cvtPeMachine() {
		uint16_t ret = 0;

		switch (config->machine) {
		case cwArchitecture::AMD64:
			ret = 0x8664; break;
		case cwArchitecture::INTEL86:
			ret = 0x14C; break;
		default:{}
		}

		return ret;
	}

	static uint64_t getNtOffset()
	{
		uint64_t sizeHeaders = config->skipDosStub ? sizeof(pe::dosHeader) : dosStubSize;
		if (!config->skipCWInfo)
			sizeHeaders += sizeof(cwDebugInformation);

		return sizeHeaders;
	}

	outputSection::outputSection(const char* name) {
		std::strcpy(secHeader.name, name);
	}

	void outputSection::addChunk(Chunk* c) {
		chunks.push_back(c);
	}

	template<typename THeader> void Linker::EmitHeader(utl::File &out)
	{
		pe::dosHeader dos{};
		dos.magic = pe::dosMagic;
		if (!config->skipDosStub) {
			dos.cblp = dosStubSize % 512;
			dos.cp = static_cast<uint16_t>(llvm::divideCeil(dosStubSize, 512));
		}

		dos.cparhdr = sizeof(pe::dosHeader) / 16;
		dos.lfarlc = sizeof(pe::dosHeader);
		dos.lfanew = static_cast<uint32_t>(getNtOffset());
		dos.maxalloc = 0xFFFF;
		dos.sp = 0x00B8;

		out.Write(dos);

		if (!config->skipDosStub) {
			out.Write(dosProgram);
		}

		if (!config->skipCWInfo) {
			g_dbgInfo.magic = 0xCE02F0; //'FORCE'
			out.Write(g_dbgInfo);
		}

		THeader nt{};
		nt.magic = pe::ntMagic;

		pe::ntFileHeader& fh = nt.fileHeader;
		fh.machine = cvtPeMachine();
		fh.numSections = static_cast<uint16_t>(outSecs.size());
		fh.timeDateStamp = config->buildTime;
		fh.sizeOptionalHeader = sizeof(nt.optionalHeader);

		fh.flags = 0x22; // todo: make changeable
		if (config->target == PeTarget::TARGET_DLL)
			fh.flags |= pe::fileFlags::FILE_DLL;

		auto& opt = nt.optionalHeader;
		opt.magic = config->machine == cwArchitecture::AMD64 ? 0x20B : 0x10B;
		opt.majorLinkerVersion = 14; // report us as vs 2019
		opt.minorLinkerVersion = 23;
		opt.majorOperatingSystemVersion = 6;
		opt.majorSubsystemVersion = 6;
		opt.imageBase = config->imageBase;
		opt.subsystem = config->subSystem;
		opt.sizeOfImage = sizeOfImage;
		opt.sizeOfHeaders = sizeHeaders;
		opt.sectionAlignment = config->align;
		opt.fileAlignment = config->fileAlignment;
		opt.numberOfRvaAndSizes = 16; //hard coded
		opt.sizeOfStackReserve = config->stackReserve;
		opt.sizeOfStackCommit = config->stackCommit;
		opt.sizeOfHeapReserve = config->heapReserve;
		opt.sizeOfHeapCommit = config->heapCommit;

		// base = first executable segment
		for (auto& it : outSecs) {
			if (it.header().flags & pe::SEC_CODE) {
				opt.baseOfCode = it.header().virtualAddress;
				break;
			}
		}

		// size of code
		for (auto& it : outSecs) {
			if (it.header().flags & pe::SEC_CODE) {
				opt.sizeCode += it.header().misc.vs;
			}
		}

		out.Write(nt);
	}

	outputSection* Linker::getSection(const char* name)
	{
		for (auto& it : outSecs) {
			if (std::strncmp(it.header().name, name, 8) == 0)
				return &it;
		}

		return nullptr;
	}

	void Linker::createSections()
	{
		constexpr uint32_t data = pe::SEC_INITIALIZED_DATA;
		constexpr uint32_t bss = pe::SEC_UNINITIALIZED_DATA;
		constexpr uint32_t code = pe::SEC_CODE;
		constexpr uint32_t discardable = pe::SEC_DISCARDABLE;
		constexpr uint32_t r = pe::SEC_READ;
		constexpr uint32_t w = pe::SEC_WRITE;
		constexpr uint32_t x = pe::SEC_EXEC;

		auto createSection = [&](const char* name, uint32_t flags)
		{
			auto& entry = outSecs.emplace_back(name);
			entry.header().flags = flags;
			return &entry;
		};

		if (config->enableDiscProtect)
			cwscSec = createSection(".cwsc", code | r | x);

		textSec = createSection(".text", code | r | x);
		createSection(".bss", bss | r | w);
		rdataSec = createSection(".rdata", data | r);
		buildidSec = createSection(".buildid", data | r);
		dataSec = createSection(".data", data | r | w);
		pdataSec = createSection(".pdata", data | r);
		idataSec = createSection(".idata", data | r);
		edataSec = createSection(".edata", data | r);
		didatSec = createSection(".didat", data | r);
		rsrcSec = createSection(".rsrc", data | r);
		relocSec = createSection(".reloc", data | discardable | r);
		ctorsSec = createSection(".ctors", data | r | w);
		dtorsSec = createSection(".dtors", data | r | w);
	}

	void Linker::removeUnusedSections()
	{
		// Remove sections that we can be sure won't get content, to avoid
		// allocating space for their section headers.
		auto isUnused = [this](outputSection& s) {
			if (&s == relocSec)
				return false; // This section is populated later.
			  // MergeChunks have zero size at this point, as their size is finalized
			  // later. Only remove sections that have no Chunks at all.
			return s.chunks.empty();
		};
		outSecs.erase(std::remove_if(outSecs.begin(), outSecs.end(), isUnused), outSecs.end());
	}

	void Linker::assignAddresses()
	{
		sizeHeaders = getNtOffset() + sizeof(pe::dataDirectory) * 16 +
			sizeof(pe::sectionHeader) * outSecs.size();
		sizeHeaders += config->is64() ? sizeof(pe::ntOptionalHeader64) : 0; /* FIX ME */

		sizeHeaders = llvm::alignTo(sizeHeaders, config->fileAlignment);
		fileSize = sizeHeaders;

		// first page
		uint64_t rva = llvm::alignTo(sizeHeaders, config->align);

		for (auto& sec : outSecs) {
			sec.header().virtualAddress = rva;

			uint64_t rawSize = 0, virtualSize = 0;

			// determine padding size
			const bool isCodeSection =
				(sec.header().flags & pe::SEC_CODE) &&
				(sec.header().flags & pe::SEC_READ) &&
				(sec.header().flags & pe::SEC_EXEC);
			uint32_t padding = isCodeSection ? config->functionPadMin : 0;

			for (Chunk* c : sec.chunks) {

				// fat padding allows for greater byte padding between functions
				if (c->useFatPadding) {
					virtualSize += padding;
				}
				virtualSize = llvm::alignTo(virtualSize, c->getAlignment());
				c->setRVA(rva + virtualSize);
				virtualSize += c->getSize();

				if (!c->data.empty())
					rawSize = llvm::alignTo(virtualSize, config->fileAlignment);
			}

			sec.header().misc.vs = virtualSize;
			sec.header().sizeOfRawData = rawSize;

			if (rawSize != 0)
				sec.header().pointerToRawData = fileSize;
			rva += llvm::alignTo(virtualSize, config->align);
			fileSize += llvm::alignTo(rawSize, config->fileAlignment);
		}

		sizeOfImage = llvm::alignTo(rva, config->align);
	}

	void Linker::writeSections(utl::File &out)
	{
		const uint8_t nullByte = 0;
		const uint8_t codeByte = 0xCC; //int3

		for (auto& sec : outSecs) {
			auto pos = out.Tell();

			// fix the gaps
			if (pos < sec.header().pointerToRawData) {
				for (int i = 0; i < sec.header().pointerToRawData - pos; i++) {
					out.Write(&nullByte, 1);
				}
			}

			const bool isCodeSection =
				(sec.header().flags & pe::SEC_CODE) &&
				(sec.header().flags & pe::SEC_READ) &&
				(sec.header().flags & pe::SEC_EXEC);

			for (uint32_t i = 0; i < sec.header().sizeOfRawData; i++) {
				out.Write(isCodeSection ? &codeByte : &nullByte, 1);
			}

			for (Chunk* c : sec.chunks) {
				uint64_t offset = sec.header().pointerToRawData + (c->getRVA() - sec.getRVA());
				std::printf("[link-chunk] O,R,SR: [%llx|%x|%llx]\n", offset, c->getRVA(), sec.getRVA());

				// move to chunk destination
				out.Seek(offset, utl::seekMode::seek_set);
				c->writeTo(out);
				out.Write(c->data);
			}
		}
	}

	bool Linker::EmitFile(utl::File &out)
	{
		removeUnusedSections();
		assignAddresses();

		if (config->is64())
			EmitHeader<pe::ntHeader64>(out);
		else {
			std::puts("FIXME: nt32 header");
		}

		for (auto& sec : outSecs) {
			out.Write(sec.header());
		}

		writeSections(out);

		return true;
	}
}