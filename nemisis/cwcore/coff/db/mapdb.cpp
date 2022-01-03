
// Copyright (C) Force67 2019

#include "mapdb.h"
#include <utl/File.h>

namespace link::coff
{
	bool mapDB::load(utl::File& file)
	{
		// load the entire file into memory
		std::vector<char> buf(file.GetSize());
		if (!file.Read(buf))
			return false;

		// initial cursor
		char* cursor = buf.data() + 1;

		auto skipLine = [&]() {
			while (*cursor++ != '\n') {}
		};

		skipLine();
		targetName = std::string(buf.data() + 1, cursor - 1);

		// skip to "preferred load addr"
		for (int ln = 0; ln < 3; ++ln) {
			skipLine();
		}

		auto readHex = [&]() {
			size_t val = 0;
			for (int i = 0; i < 16; ++i) {
				val <<= 4;
				char ch = *cursor++;
				val += ch <= '9' ? (ch - '0') : (ch - 'a' + 10);
			}
			return val;
		};

		// kinda hacky
		cursor += std::strlen(" Preferred load address is ");
		loadAddress = readHex();

		// Skip to function decelerations
		while (cursor[1] != ' ' || cursor[2] != 'A') 
			skipLine();

		while (cursor < buf.data() + file.GetSize()) {
			cursor += 21;
			const char* const nameBegin = cursor;
			while (*cursor != ' ') { ++cursor; }
			const char* const nameEnd = cursor;

			while (*cursor == ' ') { ++cursor; }
			size_t addr = readHex();
			addr -= loadAddress;

			std::string name(nameBegin, nameEnd);

			auto &node = symr.emplace_back();
			node.symName = std::move(name);
			node.symRva = addr;

			skipLine();
		}

		return true;
	}
}