#pragma once

#include <cstdint>

#ifndef _WIN32
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#else
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop) )
#endif

// supported image file types
enum class cwImageType
{
	UNKNOWN,
	IMG_PE,
	IMG_ELF,
	IMG_SELF,
	IMG_MARCH0
};

enum class cwArchitecture
{
	AMD64,
	INTEL86,
	ARM,
	ARM64
};

#include <Windows.h>

// (purpose of this is to carry
// debug information about sdk, number of markers
// etc)
struct cwDebugInformation
{
	uint32_t magic;
	uint16_t numSdkMarkers;
	uint16_t numBlueprintsUsed;
};

namespace pe
{
	constexpr uint16_t dosMagic = 0x5A4D;
	constexpr uint32_t ntMagic = 0x00004550;

	enum imageDirectoryEntry
	{
		EXPORT_DIR,
		IMPORT_DIR,
		RESOURCE_DIR,
		EXCEPTION_DIR,
		SECURITY_DIR,
		BASERELOC_DIR,
		DEBUG_DIR,
		COPYRIGHT_DIR, //x86 only
		GLOBALPTR_DIR,
		TLS_DIR,
		LOADCONFIG_DIR,
		BOUNDIMPORT_DIR,
		IAT_DIR,
		DELAYIMP_DIR,
		COMDESC_DIR
	};

	struct dosHeader
	{
		uint16_t magic;
		uint16_t cblp;
		uint16_t cp;
		uint16_t crlc;
		uint16_t cparhdr;
		uint16_t minalloc;
		uint16_t maxalloc;
		uint16_t ss;
		uint16_t sp;
		uint16_t csum;
		uint16_t ip;
		uint16_t cs;
		uint16_t lfarlc;
		uint16_t ovno;
		uint16_t res[4];
		uint16_t oemid;
		uint16_t oeminfo;
		uint16_t res2[10];
		uint32_t lfanew;
	};

	enum fileFlags : uint16_t
	{
		FILE_DLL = 0x2000,
		WORD_32_BIT = 0x0100,
	};
	struct ntFileHeader
	{
		uint16_t machine;
		uint16_t numSections;
		uint32_t timeDateStamp;
		uint32_t pointerSymbolTable;
		uint32_t numSymbols;
		uint16_t sizeOptionalHeader;
		uint16_t flags;
	};

	struct dataDirectory
	{
		uint32_t va;
		uint32_t size;
	};

	enum class debugType : uint32_t
	{
		UNKNOWN,
		COFF,
		CODEVIEW,
		FPO,
		MISC,
		EXCEPTION,
		FIXUP,
		FROM_SRC,
		BORLAND,
		RESERVED,
		CLSID,
		REPRO = 16,
		DLLFLAGS = 20
	};

	struct debugDirectory
	{
		uint32_t flags;
		uint32_t timeStamp;
		uint16_t majorVersion;
		uint16_t minorVersion;
		debugType type;
		uint32_t sizeData;
		uint32_t adressRawData;
		uint32_t ptrRawData;
	};

	struct debugCodeView_v70
	{
		uint32_t codeViewSig;
		uint8_t sig[16];
		uint32_t age;
	};

	static_assert(sizeof(debugCodeView_v70) == 24);

	struct ntOptionalHeader64
	{
		uint16_t magic;
		uint8_t majorLinkerVersion;
		uint8_t minorLinkerVersion;
		uint32_t sizeCode;
		uint32_t sizeOfInitializedData;
		uint32_t sizeOfUninitializedData;
		uint32_t addressOfEntryPoint;
		uint32_t baseOfCode;
		uint64_t imageBase;
		uint32_t sectionAlignment;
		uint32_t fileAlignment;
		uint16_t majorOperatingSystemVersion;
		uint16_t minorOperatingSystemVersion;
		uint16_t majorImageVersion;
		uint16_t minorImageVersion;
		uint16_t majorSubsystemVersion;
		uint16_t minorSubsystemVersion;
		uint32_t win32VersionValue;
		uint32_t sizeOfImage;
		uint32_t sizeOfHeaders;
		uint32_t checkSum;
		uint16_t subsystem;
		uint16_t dllCharacteristics;
		uint64_t sizeOfStackReserve;
		uint64_t sizeOfStackCommit;
		uint64_t sizeOfHeapReserve;
		uint64_t sizeOfHeapCommit;
		uint32_t loaderFlags;
		uint32_t numberOfRvaAndSizes;
		dataDirectory directories[16];
	};

	template<typename Type>
	struct ntHeader
	{
		uint32_t magic;
		ntFileHeader fileHeader;
		Type optionalHeader;
	};

	using ntHeader64 = ntHeader<ntOptionalHeader64>;

	struct sectionHeader
	{
		char name[8];
		union {
			uint32_t pa;
			uint32_t vs;
		} misc;
		uint32_t virtualAddress;
		uint32_t sizeOfRawData;
		uint32_t pointerToRawData;
		uint32_t pointerToRelocations;
		uint32_t pointerToLinenumbers;
		uint16_t numberOfRelocations;
		uint16_t numberOfLinenumbers;
		uint32_t flags;
	};

	enum subsystemType : uint16_t
	{
		SYS_GUI = 2,
		SYS_CON = 3,
	};

	enum sectionCharacteristics
	{
		SEC_EXEC = 0x20000000,
		SEC_CODE = 0x00000020,
		SEC_READ = 0x40000000,
		SEC_WRITE = 0x80000000,
		SEC_DISCARDABLE = 0x02000000,
		SEC_INITIALIZED_DATA = 0x00000040,
		SEC_UNINITIALIZED_DATA = 0x00000080
	};

	struct exportDirectory
	{
		uint32_t characteristics;
		uint32_t timeDateStamp;
		uint16_t majorVersion;
		uint16_t minorVersion;
		uint32_t name;
		uint32_t base;
		uint32_t numFunctions;
		uint32_t numNames;
		uint32_t addressOfFunctions;     // RVA from base of image
		uint32_t addressOfNames;         // RVA from base of image
		uint32_t addressOfNameOrdinals;  // RVA from base of image
	};

	enum class coffSymType : uint8_t
	{
		SYM_TYPE_NULL,
		SYM_TYPE_VOID,
		SYM_TYPE_CHAR,
		SYM_TYPE_SHORT,
		SYM_TYPE_INT,
		SYM_TYPE_LONG,
		SYM_TYPE_FLOAT,
		SYM_TYPE_DOUBLE,
		SYM_TYPE_STRUCT,
		SYM_TYPE_UNION,
		SYM_TYPE_ENUM,
		SYM_TYPE_MOE,
		SYM_TYPE_BYTE,
		SYM_TYPE_WORD,
		SYM_TYPE_UINT,
		SYM_TYPE_DWORD
	};

	enum class coffStorageclass : uint8_t
	{
		SYM_CLASS_END_OF_FUNCTION = 0xFF,
		SYM_CLASS_NULL = 0,
		SYM_CLASS_AUTOMATIC,
		SYM_CLASS_EXTERNAL,
		SYM_CLASS_STATIC,
		SYM_CLASS_REGISTER,
		SYM_CLASS_EXTERNAL_DEF,
		SYM_CLASS_LABEL,
		SYM_CLASS_UNDEFINED_LABEL,
		SYM_CLASS_MEMBER_OF_STRUCT,
		SYM_CLASS_ARGUMENT,
		SYM_CLASS_STRUCT_TAG,
		SYM_CLASS_MEMBER_OF_UNION,
		SYM_CLASS_UNION_TAG,
		SYM_CLASS_TYPE_DEFINITION,
		SYM_CLASS_UNDEFINED_STATIC,
		SYM_CLASS_ENUM_TAG,
		SYM_CLASS_MEMBER_OF_ENUM,
		SYM_CLASS_REGISTER_PARAM,
		SYM_CLASS_BIT_FIELD,
		SYM_CLASS_BLOCK = 100,
		SYM_CLASS_FUNCTION,
		SYM_CLASS_END_OF_STRUCT,
		SYM_CLASS_FILE,
		SYM_CLASS_SECTION,
		SYM_CLASS_WEAK_EXTERNAL,
		SYM_CLASS_CLR_TOKEN = 107
	};

	enum imageRelocations : uint16_t
	{
		REL_AMD64_ABSOLUTE,
		REL_AMD64_ADDR64,
		REL_AMD64_ADDR32,
		REL_AMD64_ADDR32NB,
		REL_AMD64_REL32,
		REL_AMD64_REL32_1,
		REL_AMD64_REL32_2,
		REL_AMD64_REL32_3,
		REL_AMD64_REL32_4,
		REL_AMD64_REL32_5,
		REL_AMD64_SECREL = 0x000B,
	};

	PACK (struct CoffSym
	{
		union {
			char name[8];
			struct {
				uint32_t nameZeroes;
				uint32_t dontAnnoyMe;
			};
		};
		uint32_t value;
		int16_t secNum; // WARNING: this index is one based
		uint8_t complexType;
		coffSymType baseType;
		coffStorageclass storageClass;
		uint8_t numAuxSymbols;
	});

	PACK (struct CoffReloc
	{
		uint32_t vAddr;
		uint32_t symTabIdx;
		imageRelocations type;
	});

	static_assert(sizeof(CoffSym) == 18);
	static_assert(sizeof(CoffReloc) == 10);
}

namespace elf
{
	constexpr uint32_t elfMagic = 0x464C457F;
}
