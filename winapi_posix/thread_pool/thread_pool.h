#include <atomic>
#include <vector>
#include <thread>
#include <functional>
#include <iostream>

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
#ifdef _WIN32
  std::vector<HANDLE> &threads;
#elif __linux__
  std::vector<pthread_t> &threads;
#endif

public:
#ifdef _WIN32
  explicit join_threads(std::vector<HANDLE> &threads_) : threads(threads_) {}
#elif __linux__
  explicit join_threads(std::vector<pthread_t> &threads_) : threads(threads_) {}
#endif

  ~join_threads()
  {
#ifdef _WIN32
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
};

// custom thread_pool implementation using threadsafe queue
class thread_pool
{
private:
  std::atomic_bool done;
  threadsafe_queue<std::function<void()>> work_queue;
  // join_threads joiner;
#ifdef _WIN32
  std::vector<HANDLE> threads;
#elif __linux__
  std::vector<pthread_t> threads;
#endif

  uint32_t thread_count;

#ifdef _WIN32
  static unsigned int __stdcall worker_thread(void *param);
#elif __linux__
  static void *worker_thread(void *param);
#endif

public:
  thread_pool();
  ~thread_pool();

  thread_pool &operator=(thread_pool &&) = delete;
  thread_pool(thread_pool &&) = delete;

  thread_pool(const thread_pool &) = delete;
  thread_pool &operator=(const thread_pool &) = delete;

  size_t get_threads_count();
  void submit(const std::function<void()> &f);
};