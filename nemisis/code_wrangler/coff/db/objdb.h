#pragma once

// Copyright (C) Force67 2019

#include <fmtdef.h>
#include <string>
#include "../chunk.h"

namespace utl {
	class File;
}

namespace link::coff
{
	enum class coffObjErr
	{
		none,
		badmagic,
		badsec,
	};

	class coffObj
	{
	public:

		coffObj() = default;
		explicit coffObj(const char*);
	
		coffObjErr load(utl::File&);
		SectionChunk* getObject(const char* name);
		SectionChunk* getFunc(const char* name);

		inline std::vector<SectionChunk>& getSections() { return sections; }
		inline std::vector<pe::CoffSym>& getSymbols() { return symbols; }
	
		inline std::string name() {
			return objName;
		}

	private:
		std::string objName;

		struct funcRecord {
			SectionChunk* data;
			const char* name;
		};

		std::vector<SectionChunk> sections;
		std::vector<pe::CoffSym> symbols;
		std::vector<uint8_t> strTab;
		std::vector<funcRecord> funcs;
		pe::ntFileHeader header{};
	};
}