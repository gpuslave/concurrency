#include "thread_pool.h"
#include <system_error>

thread_pool::thread_pool(std::mutex *cout_mut) : done(false)
{
  unsigned long const hardware_threads = std::thread::hardware_concurrency();

  thread_count = hardware_threads != 0 ? hardware_threads : 2;
  try
  {
    DWORD last_thread_err;
    for (uint32_t i = 0; i < thread_count; ++i)
    {
#ifdef _WIN32
      HANDLE newthread = (HANDLE)_beginthreadex(nullptr, 0, &thread_pool::worker_thread, this, 0, nullptr);

      last_thread_err = GetLastError();

      {
        std::lock_guard<std::mutex> cout_lk(*cout_mut);
        std::cout << last_thread_err << " - " << std::system_category().message(last_thread_err) << std::endl;
      }

      if (newthread == nullptr)
      {
        std::cerr << "Thread creation using _beginthreadex failed with " << last_thread_err << " error code." << std::endl;
      }
      else
      {
        threads.push_back(newthread);
      }

#elif __linux__
      pthread_t thread;

      int err{0};
      err = pthread_create(&thread, nullptr, &thread_pool::worker_thread, this);
      if (err != 0)
      {
        std::cerr << "Thread creation using pthread_create failed with " << err << " error code." << std::endl;
      }
      else
      {
        threads.push_back(thread);
      }
#endif
    }
    if (!threads.size())
    {
      std::cerr << "All threads failed";
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
#ifdef _WIN32
  std::cout << std::endl
            << threads.size() << std::endl;
  WaitForMultipleObjects(threads.size(), threads.data(), TRUE, INFINITE);

  for (uint32_t i = 0; i < threads.size(); ++i)
  {
    CloseHandle(threads[i]);
  }
#elif __linux__
  for (uint32_t i = 0; i < threads.size(); ++i)
  {
    std::cout << i << " ";
    pthread_join(threads[i], nullptr);
  }
#endif
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