// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.
// Entry point of the sync plugin

#include "Plugin.h"
#include "utils/Logger.h"

namespace noda
{
	Plugin::Plugin() :
	    _syncController(),
	    _uiController(_syncController)
	{
		LOG_INFO("Loaded NODA, version " GIT_BRANCH "@" GIT_COMMIT " Copyright(c) NOMAD Group <nomad-group.net>.");
#if 0

		std::string result;
		result.reserve(32);

		uchar md5[16];
		retrieve_input_file_md5(md5);

		for (int i = 0; i < 16; i++)
		{
			result += "0123456789ABCDEF"[md5[i] / 16];
			result += "0123456789ABCDEF"[md5[i] % 16];
		}

		char buf[512]{};
		get_root_filename(buf, 512);

		LOG_TRACE("Input file md5 {} - {}", result, buf);
#endif
	}

	Plugin::~Plugin()
	{
	}
} // namespace noda

namespace
{
	using namespace noda;

	Plugin *g_Plugin = nullptr;

	const char kPluginComment[] = "Nomad Ida Plugin";
	const char kPluginName[] = "NODA";
	const char kPluginHotkey[] = "Alt-L";

	int idaapi PluginInit()
	{
		g_Plugin = new Plugin();
		return PLUGIN_KEEP;
	}

	void idaapi PluginShutdown()
	{
		if(g_Plugin)
			delete g_Plugin;
	}

	bool idaapi PluginRun(size_t arg)
	{
		return true;
	}
} // namespace

plugin_t PLUGIN = {
	IDP_INTERFACE_VERSION,
	PLUGIN_FIX, //< Load the plugin when ida starts
	PluginInit,
	PluginShutdown,
	PluginRun,
	kPluginComment,
	kPluginComment,
	kPluginName,
	kPluginHotkey
};