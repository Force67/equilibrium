
// Copyright (C) Force67 2019

#include "cwcore.h"
#include "coff/linkdriver.h"

namespace link::coff {
	extern Configuration* config;
}

// choose image type based on image magic
cwImageType cwChooseImageType(const char *inputFile)
{
	utl::File file(inputFile);
	if (!file.IsOpen())
		return cwImageType::UNKNOWN;

	uint32_t magic = 0;
	file.Read(magic);

	if (*(uint16_t*)&magic == pe::dosMagic)
		return cwImageType::IMG_PE;

	if (magic == elf::elfMagic)
		return cwImageType::IMG_ELF;

	return cwImageType::UNKNOWN;
}

bool cwLinkImage(cwImageType type, const char *outputFile, const char *commands)
{
	utl::File inFile(outputFile, utl::fileMode::write);
	if (!inFile.IsOpen())
		return false;

	switch (type) {
	case cwImageType::IMG_PE: {
		return link::coff::linkPe({}, inFile);
	}
	}

	return false;
}

bool cwRelinkImage(cwImageType type, const char* inputFile, const char* outputDir, const char* outputFileName)
{
	using namespace link::coff;

	if (!inputFile || !outputDir)
		return false;

	std::string inF(inputFile);
	std::string outDir(outputDir);

	if (!outputFileName) {

		// extract just the targetname
		size_t pos = inF.find_last_of('\\') + 1;
		auto targetName = inF.substr(pos, inF.length() - pos);
		config->inputName = targetName;

		pos = targetName.find_last_of('.');
		targetName.insert(pos, "_cw");

		outDir += "\\" + targetName;
	}
	else {
		outDir += "\\" + std::string(outputFileName);
	}

	config->rootDir = inF.substr(0, inF.find_last_of("//"));
	config->inputFile = std::move(inputFile);
	config->outputFile = std::move(outDir);

	switch (type) {
	case cwImageType::IMG_PE: {
		return link::coff::relinkPe();
	}
	}

	return false;
}