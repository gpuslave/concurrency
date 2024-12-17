#include <atomic>
#include <vector>
#include <thread>
#include <functional>

#ifdef _WIN32
#include <Windows.h>
#elif __linux__
#include <pthread.h>
#endif

#include "threadsafe_queue.h"

// helper class
class join_threads
{
private:
  std::vector<std::thread> &threads;

public:
  explicit join_threads(std::vector<std::thread> &threads_) : threads(threads_) {}

  ~join_threads()
  {
    for (unsigned long i = 0; i < threads.size(); ++i)
    {
      if (threads[i].joinable())
        threads[i].join();
    }
  }
};

// custom thread_pool implementation using threadsafe queue
class thread_pool
{
private:
  std::atomic_bool done;
  threadsafe_queue<std::function<void()>> work_queue;
  join_threads joiner;
#ifdef _WIN32
  std::vector<HANDLE> threads;
#elif __linux__
  std::vector<pthread_t> threads;
#endif

  unsigned long thread_count;
  void worker_thread();

#ifdef _WIN32
  static unsigned int __stdcall thread_wrapper(void *param)
  {
    thread_pool *pool = static_cast<thread_pool *>(param);
    try
    {
      pool->worker_thread();
    }
    catch (...)
    {
    }

    return 0;
  }
#elif __linux__
#endif

public:
  thread_pool();
  ~thread_pool();

  thread_pool(thread_pool &&other) : done(other.done.load()),
                                     work_queue(std::move(other.work_queue)),
                                     threads(std::move(other.threads)),
                                     joiner(threads),
                                     thread_count(other.thread_count)
  {
    other.done = true;
  }

  thread_pool(const thread_pool &) = delete;
  thread_pool &operator=(const thread_pool &) = delete;

  size_t get_threads_count();
  void submit(const std::function<void()> &f);
};