#pragma once

#include <cstdint>

namespace pdb
{
	namespace v2
	{
		struct Header
		{
			char signature[44];
			uint32_t pageSize;
			uint16_t startPage;
			uint16_t numPagesizes;
			uint32_t rootStreamSize;
			uint32_t reserved;
		};

		static_assert(sizeof(Header) == 60);
	}

	namespace v7
	{
		struct Header
		{
			char signature[32];
			uint32_t pageSize;
			uint32_t ptrAllocTab;
			uint32_t numFilePages;
			uint32_t rootStreamSize;
			uint32_t reserved;
		};

		static_assert(sizeof(Header) == 52);
	}
}