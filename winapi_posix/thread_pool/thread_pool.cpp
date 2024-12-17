#include "thread_pool.h"

thread_pool::thread_pool() : done(false), joiner(threads)
{
  unsigned long const hardware_threads = std::thread::hardware_concurrency();

  thread_count = hardware_threads != 0 ? hardware_threads : 2;
  try
  {
    for (unsigned i = 0; i < thread_count; ++i)
    {
#ifdef _WIN32
      HANDLE newthread = (HANDLE)_beginthreadex(nullptr, 0, &thread_pool::worker_thread, this, 0, nullptr);
      threads.push_back(newthread);
#elif __linux__
      pthread_t thread;
      pthread_create(&thread, nullptr, &thread_pool::worker_thread, this);
      threads.push_back(thread);
#endif
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

#ifdef _WIN32
unsigned int __stdcall thread_pool::worker_thread(void *param)
#elif __linux__
void *thread_pool::worker_thread(void *param)
#endif
{
  thread_pool *pool = static_cast<thread_pool *>(param);
  while (!pool->done)
  {
    std::function<void()> task;
    if (pool->work_queue.try_pop(task))
    {
      task();
    }
    else
    {
      std::this_thread::yield();
    }
  }
#ifdef _WIN32
  _endthreadex(0);
  return 0;
#elif __linux__
  return nullptr;
#endif
}

size_t thread_pool::get_threads_count()
{
  return thread_count;
}

void thread_pool::submit(const std::function<void()> &f)
{
  work_queue.push(std::function<void()>(f));
}