#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <ida.hpp>
#include <idp.hpp>

namespace noda {
  class JobQueue {
  public:
	using function_t = std::function<void()>;

	struct Executer : exec_request_t {
	  std::mutex mutex;

	  std::queue<function_t> jobs;
	  void Enqueue(const function_t &);
	  function_t Dequeue();

	  virtual int idaapi execute() override;
	} _executer;

  public:
	JobQueue() = default;
	~JobQueue() = default;

	void ScheduleJob(const function_t&);
  };
} // namespace noda