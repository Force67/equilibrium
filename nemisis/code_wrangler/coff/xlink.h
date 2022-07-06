#pragma once

// Copyright (C) Force67 2019

#include <utl/File.h>
#include <fmtdef.h>

#include "cwcore.h"
#include "chunk.h"
#include "linker.h"
#include "transform.h"

#include "db/ldb.h"
#include "db/mapdb.h"
#include "db/pdbdb.h"

namespace link::coff
{
	class xLink
	{
		friend class coff::Linker;

	public:
		xLink();

		bool loadInputs();
		bool process();
		bool link();

	private:
		void inheritInformation();

		ldb database;

		std::unique_ptr<mapDB> mapInfo;
		std::unique_ptr<pdbObj> pdbInfo;

		std::unique_ptr<Linker> linkm;
		std::unique_ptr<codeTransform> codetf;
	};
}