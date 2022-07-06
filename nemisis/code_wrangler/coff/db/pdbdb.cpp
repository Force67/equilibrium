
// Copyright (C) Force67 2019

#include "pdbdb.h"

#include <fmtdef.h>
#include <utl/File.h>

namespace link::coff
{
	pdbObjError pdbObj::load(utl::File& file)
	{
		pdb::v7::Header header{};
		file.Read(header);

		if (std::strncmp(header.signature, "Microsoft C/C++ MSF 7.00", 24) == 0)
			return pdbObjError::badversion;



		return pdbObjError::ok;
	}
}