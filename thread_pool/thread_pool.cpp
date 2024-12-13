#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include "threadsafe_queue.cpp"

class join_threads
{
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

class thread_pool
{
private:
  std::atomic_bool done;
  threadsafe_queue<std::function<void()>> work_queue;
  std::vector<std::thread> threads;
  join_threads joiner;

  unsigned long thread_count;

  void worker_thread()
  {
    while (!done)
    {
      std::function<void()> task;
      if (work_queue.try_pop(task))
      {
        task();
      }
      else
      {
        std::this_thread::yield();
      }
    }
  }

public:
  thread_pool() : done(false), joiner(threads)
  {
    unsigned long const hardware_threads = std::thread::hardware_concurrency();

    thread_count = hardware_threads != 0 ? hardware_threads : 2;
    try
    {
      for (unsigned i = 0; i < thread_count; ++i)
      {
        threads.push_back(std::thread(&thread_pool::worker_thread, this));
      }
    }
    catch (...)
    {
      done = true;
      throw;
    }
  }
  ~thread_pool()
  {
    done = true;
  }

  size_t get_threads_count()
  {
    return thread_count;
  }

  void submit(const std::function<void()> &f)
  {
    work_queue.push(std::function<void()>(f));
  }
};