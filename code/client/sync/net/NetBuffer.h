// NODA: Copyright(c) NOMAD Group<nomad-group.net>

#pragma once
#include "../netmessages/MsgList_generated.h"

#if 0
template<typename T>
class NetPacket {
  public:
  NetPacket(netmsg::Data typeId, flatbuffers::Offset<void> data) {
	_root = netmsg::CreateMessageRoot(_fbb, typeId, data);
  }

  const char* name() const {
	const netmsg::MessageRoot *msgtest =
		netmsg::GetMessageRoot(_fbb.GetBufferPointer());

	return netmsg::EnumNameData(msgtest->data_type());
  }

  ~NetPacket() {
	_fbb.Finish(_root);
  }

  private:
  flatbuffers::Offset<netmsg::MessageRoot> _root;
  flatbuffers::FlatBufferBuilder _fbb;
};
#endif