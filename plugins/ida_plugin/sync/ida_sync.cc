// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ida_sync.h"
#include "message_handler.h"

#include <QSettings>
#include <sync/utils/user_info.h>
#include <sync/protocol/generated/message_root_generated.h>

IdaSync::IdaSync(Plugin& plugin)
    : client_(*this),
      netRunner_(client_),
      reqRunner_(*this),
      idaHandler_(*this),
      plugin_(plugin) {
  BindStaticHandlers();
  state_ = State::kDisabled;

  // after timeout, we reset the network.
  QObject::connect(&timer_, &QTimer::timeout, this,
                   [&]() { SetState(State::kDisabled); });
}

IdaSync::~IdaSync() {
  state_ = State::kDisabled;
}

void IdaSync::BindStaticHandlers() {
  size_t count = 0;

  for (auto* i = sync::StaticHandler::ROOT(); i;) {
    if (auto* it = i->item) {
      const IdaEventType_t pair = {it->hookType, it->hookEvent};

      idaEvents_[pair] = it;
      netEvents_[it->msgType] = it;
    }

    auto* j = i->next;
    i = j;

    ++count;
  }

  LOG_TRACE("BindStaticHandlers() -> count: {}", count);
}

void IdaSync::SetState(State newState) {
  if (newState != state_) {
    LOG_TRACE("Updated transport state {} -> {}",
              static_cast<int>(state_),
              static_cast<int>(newState));

    state_ = newState;

    switch (newState) { 
    case State::kActive:

    }

    emit StateChange(state_);
  }
}

// send handshake -> enter pending state for up to 5 seconds
void IdaSync::OnConnection(const sockpp::inet_address& address) {
  std::string username;
  {
    QSettings settings;
    username = settings.value("RETK_SyncUser", "").toString().toUtf8();
  }

  if (username.empty())
    username = sync::utils::GetSysUserName();

  auto request = protocol::CreateHandshakeRequest(
      fbb_, network::kClientVersion, fbb_.CreateSharedString(""),
      fbb_.CreateString(sync::utils::GetUniqueUserId()),
      fbb_.CreateString(username));

  _plugin.client().SendPacket(protocol::MsgType_HandshakeRequest, buffer,
                              request.Union());

  SetState(State::kPending);
  // and we give the timer 5 seconds to respond
  timer_.start(5000);
}

void IdaSync::OnDisconnected(int reason) {
  SetState(State::kDisabled);
}

void IdaSync::HandleAuthAck(const protocol::MessageRoot* root) {
  timer_.stop();
  SetState(State::kActive);

  const protocol::HandshakeAck* msg = root->msg_as_HandshakeAck();

  userCount = msg->numUsers();
  LOG_INFO("Joined session. {} users online.", msg->numUsers());

  emit Notify(Notification::kUsersJoined);
}

void IdaSync::HandleUserEvent(const protocol::MessageRoot* root) {
  const protocol::UserEvent* msg = root->msg_as_UserEvent();

  LOG_TRACE("HandleUserEvent() -> {}",
            protocol::EnumNameUserEventType(msg->type()));

  if (msg->type() == protocol::UserEventType_Join) {
    userCount++;
    emit Notify(Notification::kUserJoined);
  }

  if (msg->type() == protocol::UserEventType_Quit) {
    if (userCount - 1 < 1) {
      LOG_WARNING(
          "HandleUserEvent() -> NetCrime: less than one user not possible");
      return;
    }

    userCount--;
    emit Notify(Notification::kUserQuit);
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


/*

network::FbsStringRef CreateFbStringRef(network::FbsBuffer& buffer,
                                        const QString& other) {
  const char* str = const_cast<const char*>(other.toUtf8().data());
  size_t sz = static_cast<size_t>(other.size());
  return buffer.CreateString(str, sz);
}

*/

/*

void SyncService::SendSessionInfo() {
  assert(!IsOnNetThread());

  LOG_TRACE("SendSessionInfo() -> md5: {} fileName: {}", md5Str, fileName);

  network::FbsBuffer buffer;
  auto request = protocol::CreateLocalProjectInfoDirect(
      buffer, md5Str, fileName, _session.Store().LocalVersion());

  GNetClient()->SendPacket(protocol::MsgType_LocalProjectInfo, buffer,
                           request.Union());
}
*/

/*

void SyncSession::LoginUser() {
  QString name, guid = sync_utils::GetUserGuid();

  {
    QSettings settings;
    name = settings.value("Nd_SyncUser", sync_utils::GetDefaultUserName())
               .toString();
  }

  network::FbsBuffer buffer;
  auto request = protocol::CreateHandshakeRequest(
      buffer, network::kClientVersion, buffer.CreateSharedString(""),
      sync_utils::CreateFbStringRef(buffer, guid),
      sync_utils::CreateFbStringRef(buffer, name));

  _plugin.client().SendPacket(protocol::MsgType_HandshakeRequest, buffer,
                              request.Union());

  SetTransportState(TransportState::PENDING);

  // give the server 5 seconds to respond
  _timeout->start(5000);

  // TODO: think about making timeout stuff a client feature...
  _timeout->moveToThread(&_plugin.client());
}
*/