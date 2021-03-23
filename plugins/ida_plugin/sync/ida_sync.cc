// Copyright (C) Force67 <github.com/Force67>.
// For licensing information see LICENSE at the root of this distribution.

#include "ida_sync.h"
#include "message_handler.h"
#include "utils/ida_plus.h"

#include <QSettings>
#include <sync/utils/user_info.h>
#include <sync/protocol/generated/message_root_generated.h>

IdaSync::IdaSync(Plugin& plugin)
    : client_(*this),
      netRunner_(client_),
      reqRunner_(*this),
      idaHandler_(*this),
      plugin_(plugin),
      context_(*this) {
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
    LOG_TRACE("Updated transport state {} -> {}", static_cast<int>(state_),
              static_cast<int>(newState));

    state_ = newState;

    if (newState == State::kActive) {
      std::function<void(void)> func =
          std::bind(&IdaSync::SendProjectInfo, this);
      utils::RequestFunctor functor(func, MFF_READ);
    }

    emit StateChange(state_);
  }
}

// send handshake -> enter pending state for up to 5 seconds
// no need to execute on ida thread
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

  client_.Send(fbb_, protocol::MsgType_HandshakeRequest, request.Union());

  SetState(State::kPending);
  // and we give the timer 5 seconds to respond
  timer_.start(5000);
  // i don't even remember why i did this
  timer_.moveToThread(&netRunner_);
}

void IdaSync::OnDisconnected(int reason) {
  SetState(State::kDisabled);
}

void IdaSync::SendProjectInfo() {
  auto md5 = utils::InputFile::RetrieveInputFileNameMD5();
  auto name = utils::InputFile::GetInputFileName();

  LOG_TRACE("SendProjectInfo() -> md5: {} fileName: {}", md5, name);

  auto request = protocol::CreateLocalProjectInfoDirect(
      fbb_, md5.c_str(), name.c_str(), data_.version_);

  client_.Send(fbb_, protocol::MsgType_LocalProjectInfo, request.Union());
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

void IdaSync::ConsumeMessage(const protocol::MessageRoot* root, size_t len) {
  LOG_TRACE("ConsumeMessage() -> {}",
            protocol::EnumNameMsgType(root->msg_type()));

  switch (root->msg_type()) {
    case protocol::MsgType_HandshakeAck:
      return HandleAuthAck(root);
    case protocol::MsgType_UserEvent:
      return HandleUserEvent(root);
    default:
      // dispatch the message to the IDA thread
      reqRunner_.Queue(static_cast<const uint8_t*>(root->msg()), len);
      break;
  }
}