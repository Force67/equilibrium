#pragma once

// Copyright (C) Force67 2019

#include "disasm.h"
#include "chunk.h"

#include "db/ldb.h"
#include "db/objdb.h"

namespace link::coff
{
	class Linker;

	enum class codeMarkerType {
		MARKER_SPOT,
		MARKER_TRIGGER
	};

	struct codeMarker {
		codeMarkerType markerType;
		uint32_t markerAddr;
		uint32_t markerSize;
		const char* strDesc;
	};

	class codeTransform : public disasm
	{
	public:
		codeTransform(ldb&, Linker*);

		bool init();
		bool disasmSection(Section&);
		void mergeBlueprints();

		inline std::vector<codeMarker> &getCodeMarkers() {
			return codeMarkers;
		}

	private:

		bool Diasm_AMD64(const uint8_t*);
		bool Diasm_X86();

		bool loadBlueprint(const char* name);

		ldb& db;

		std::vector<coffObj> blueprints;

		uint32_t spotmarkerAddr = 0;
		uint32_t triggermarkerAddr = 0;
		uint32_t codemarkerStart = 0;

		bool markerInfofetched = false;
		codeMarker* curMarker = nullptr;
		std::vector<codeMarker> codeMarkers;

		Linker* link = nullptr;
	};
}