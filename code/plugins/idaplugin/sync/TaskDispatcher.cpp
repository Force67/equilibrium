// Copyright (C) NOMAD Group <nomad-group.net>.
// For licensing information see LICENSE at the root of this distribution.

#include "Pch.h"
#include "SyncController.h"

#include "TaskHandler.h"
#include "TaskDispatcher.h"

namespace noda {

  TaskDispatcher::TaskDispatcher(SyncController &sc) :
      _sc(sc)
  {
	for(auto *i = TaskHandler::ROOT(); i;) {
	  if(TaskHandler *it = i->handler) {
		_idaEvents[std::make_pair(it->hookType, it->hookEvent)] = it;
		_netEvents[it->msgType] = it;
	  }

	  auto *j = i->next;
	  i->next = nullptr;
	  i = j;
	}
  }

  void TaskDispatcher::DispatchEvent(hook_type_t type, int code, va_list args)
  {
	const auto it = _idaEvents.find(std::make_pair(type, code));
	if(it != _idaEvents.end()) {
	  it->second->delegates.react(_sc, args);
	}
  }

  void TaskDispatcher::QueueTask(const uint8_t *data, size_t size)
  {
	Task *task = _taskPool.allocate();
	task->key.next = nullptr;
	task->data = std::make_unique<uint8_t[]>(size);

	std::memcpy(task->data.get(), data, size);
	_taskQueue.push(&task->key);

	// flush
	_eventSize++;
	if(_eventSize == 1)
	  execute_sync(*this, MFF_WRITE | MFF_NOWAIT);
  }

  int TaskDispatcher::execute()
  {
	// Executes in IDA thread..

	while(Task *item = _taskQueue.pop(&Task::key)) {
	  _eventSize--;

	  // unpack the message
	  const protocol::MessageRoot *message = protocol::GetMessageRoot(item->data.get());

	  auto it = _netEvents.find(message->msg_type());
	  if(it != _netEvents.end()) {
		it->second->delegates.apply(_sc, message->msg());
	  }

	  _taskPool.destruct(item);
	}

	return 0;
  }
} // namespace noda