#include "JobQueue.h"

namespace noda {
  void JobQueue::Executer::Enqueue(const function_t &job)
  {
	std::lock_guard<std::mutex> lock(mutex);
	jobs.push(job);
  }

  JobQueue::function_t JobQueue::Executer::Dequeue()
  {
	std::lock_guard<std::mutex> lock(mutex);
	const auto job = jobs.back();
	jobs.pop();
	return job;
  }

  int JobQueue::Executer::execute()
  {
	Dequeue()();
	return 0;
  }

  void JobQueue::ScheduleJob(const function_t &job)
  {
	_executer.Enqueue(job);
	execute_sync(_executer, MFF_WRITE | MFF_NOWAIT);
  }
} // namespace noda