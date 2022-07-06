#pragma once

// Copyright (C) Force67 2019

#include <utl/File.h>
#include <fmtdef.h>

#include "../chunk.h"

namespace link::coff
{
	/*
		ldb holds runtime linkage information
	*/

	enum class ldbError
	{
		OK,
		BADMAGIC,
		SEGERR,
	};

	class ldb
	{
	public:

		bool deserialize(utl::File& in);
		ldbError loadPe(utl::File& peFile);

		uint8_t* getDataPtr(uint32_t rva);
		uint32_t getOfs(uint32_t rva);

		uint8_t* getVirtualPtr(uint32_t ofs);
		uint32_t getExportRva(const char* name);

		pe::ntHeader64 getNt() { return ntHdr; }

		inline bool isDll() const {
			return ntHdr.fileHeader.flags & pe::fileFlags::FILE_DLL;
		}

		std::string& getPdbDir() { return pdbDir; }

	private:

		template<typename Type>
		Type* makeOffset(const uint32_t rva) {
			return reinterpret_cast<Type*>(getDataPtr(rva));
		}

		pe::dosHeader dosHdr{};
		pe::ntHeader64 ntHdr{};
		std::string pdbDir;
		std::unique_ptr<uint8_t> ntHeaderdata;
		std::vector<Section> sections;

		friend class xLink;
	};
}