// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ida_sync_client.h"
#include "utils/logger.h"

#include <sync/protocol/generated/message_root_generated.h>

IDASyncClient::IDASyncClient() : 
	client_(*this), netRunner_(client_), reqRunner_(client_), idaHandler_(client_) {}

IDASyncClient::~IDASyncClient() = default;

void IDASyncClient::OnConnection(const sockpp::inet_address&) {
  
}

void IDASyncClient::OnDisconnected(int reason) {
  
}

void IDASyncClient::HandleAuthAck(const protocol::MessageRoot* root) {
  timer_.stop();
  //SetTransportState(TransportState::ACTIVE);

  const protocol::HandshakeAck* msg = root->msg_as_HandshakeAck();

  userCount = msg->numUsers();
  LOG_INFO("Joined session. {} users online.", msg->numUsers());

  //emit SessionNotification(NotificationCode::USERS_JOIN);
}

void IDASyncClient::HandleUserEvent(const protocol::MessageRoot* root) {
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

void IDASyncClient::ConsumeMessage(const protocol::MessageRoot *root) {
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
