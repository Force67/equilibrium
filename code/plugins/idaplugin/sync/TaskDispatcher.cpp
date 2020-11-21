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

	  const protocol::Message *message =
	      protocol::GetMessage(item->data.get());

	  switch(message->msg_type()) {
	  default:
		break;
	  }

	  if(auto *handler = _sc.HanderByNetType(message->msg_type())) {
		handler->delegates.apply(_sc, message->msg());
	  }

	  _taskPool.destruct(item);
	}

	return 0;
  }
} // namespace noda