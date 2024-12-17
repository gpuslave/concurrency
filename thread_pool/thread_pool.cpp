#include "thread_pool.h"

thread_pool::thread_pool() : done(false), joiner(threads)
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

thread_pool::~thread_pool()
{
  done = true;
}

void thread_pool::worker_thread()
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

size_t thread_pool::get_threads_count()
{
  return thread_count;
}

void thread_pool::submit(const std::function<void()> &f)
{
  work_queue.push(std::function<void()>(f));
}