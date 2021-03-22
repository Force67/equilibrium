// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ida_sync.h"
#include "message_handler.h"
#include "utils/logger.h"

#include <sync/protocol/generated/message_root_generated.h>

IdaSync::IdaSync(Plugin& plugin) : 
	client_(*this),
    netRunner_(client_),
    reqRunner_(client_),
    idaHandler_(client_),
    plugin_(plugin),{
  BindStaticHandlers();
}

IdaSync::~IdaSync() = default;

void IdaSync::BindStaticHandlers() {
  size_t count = 0;

  for (auto* i = sync::StaticHandler::ROOT(); i;) {
    if (auto* it = i->item) {
      idaEvents_[std::make_pair(it->hookType, it->hookEvent)] = it;
      netEvents_[it->msgType] = it;
    }

    auto* j = i->next;
    i = j;

    ++count;
  }

  LOG_TRACE("BindStaticHandlers() -> count: {}", count);
}

void IdaSync::OnConnection(const sockpp::inet_address&) {
  
}

void IdaSync::OnDisconnected(int reason) {
  
}

void IdaSync::HandleAuthAck(const protocol::MessageRoot* root) {
  timer_.stop();
  //SetTransportState(TransportState::ACTIVE);

  const protocol::HandshakeAck* msg = root->msg_as_HandshakeAck();

  userCount = msg->numUsers();
  LOG_INFO("Joined session. {} users online.", msg->numUsers());

  //emit SessionNotification(NotificationCode::USERS_JOIN);
}

void IdaSync::HandleUserEvent(const protocol::MessageRoot* root) {
  const protocol::UserEvent* msg = root->msg_as_UserEvent();

  LOG_TRACE("HandleUserEvent() -> {}",
            protocol::EnumNameUserEventType(msg->type()));

  if (msg->type() == protocol::UserEventType_Join) {
    userCount++;
    //emit SessionNotification(NotificationCode::USER_JOIN);
  }

  if (msg->type() == protocol::UserEventType_Quit) {
    if (userCount - 1 < 1) {
      LOG_WARNING(
          "HandleUserEvent() -> NetCrime: less than one user not possible");
      return;
    }

    userCount--;
    //emit SessionNotification(NotificationCode::USER_QUIT);
  }
}

void IdaSync::ConsumeMessage(const protocol::MessageRoot *root) {
  LOG_TRACE("ConsumeMessage() -> {}",
            protocol::EnumNameMsgType(root->msg_type()));

   switch (root->msg_type()) {
    case protocol::MsgType_HandshakeAck:
       return HandleAuthAck(root);
    case protocol::MsgType_UserEvent:
      return HandleUserEvent(root);
    default:
      break;
  }


   reqRunner_.Queue(root->msg(), root->
   /*
   if (blah)
    data_.version_++;
  */
}
