#pragma once

// Copyright (C) Force67 2019

#include <pdbspec.h>

namespace utl {
	class File;
}

namespace link::coff
{
	enum class pdbObjError
	{
		ok,
		badversion,
	};

	class pdbObj
	{
	public:

		pdbObjError load(utl::File&);
	};
}