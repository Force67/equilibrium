
// Copyright (C) Force67 2019

#include <cwcore.h>
#include <cstdio>
#include <utl/File.h>

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::puts("Usage: cwf <path>");
		return -1;
	}

	std::printf("Opening image %s\n", argv[1]);

	// auto choose type...
	const char* typeStr = nullptr;

	auto type = cwChooseImageType(argv[1]);
	switch (type) {
	case cwImageType::IMG_PE: typeStr = "PE Image"; break;
	case cwImageType::IMG_ELF: typeStr = "ELF Image"; break;
	case cwImageType::IMG_SELF: typeStr = "Signed ELF Image"; break;
	case cwImageType::IMG_MARCH0: typeStr = "March0 Image"; break;
	}

	if (typeStr)
		std::printf("Auto selected type %s\n", typeStr);

	bool result = cwRelinkImage(type, argv[1], R"(H:\.projects\codewrangler\bin\Debug)", nullptr);
	if (!result) {
		std::puts("Failed to relink image!");
	}
}