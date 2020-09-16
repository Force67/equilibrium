// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Dispatcher.h"

#include "IdaInc.h"
#include <name.hpp>

namespace noda::sync_NameAddr
{
  using namespace protocol::sync;

  bool Apply(void *IdbInstance, const protocol::MessageRoot &root)
  {
	const auto *pack = root.msg_as_sync_NameAddr();

	return set_name(
	    static_cast<ea_t>(pack->addr()),
	    pack->name()->c_str(),
	    (pack->local() ? SN_LOCAL : 0) | SN_NOWARN);

	//msg("%x was named %s\n", pack->addr(), pack->name()->c_str());
  }

  bool React(net::NetClient &sender, va_list list)
  {
	auto addr = static_cast<uint64_t>(va_arg(list, ea_t));
	auto *name = va_arg(list, const char *);
	auto local = va_arg(list, int) != 0;

	net::FbsBuilder fbb;
	auto pack = CreateNameAddr(fbb,
	                           addr,
	                           fbb.CreateString(name),
	                           static_cast<bool>(local));

	return sender.SendFBReliable(fbb, protocol::MsgType_sync_NameAddr, pack);
  }
} // namespace noda::sync_NameAddr