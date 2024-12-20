#include "thread_pool.h"

thread_pool::thread_pool() : done(false) // , joiner(threads)
{
  unsigned long const hardware_threads = std::thread::hardware_concurrency();

  thread_count = hardware_threads != 0 ? hardware_threads : 2;
  try
  {
    for (uint32_t i = 0; i < thread_count; ++i)
    {
#ifdef _WIN32
      HANDLE newthread = (HANDLE)_beginthreadex(nullptr, 0, &thread_pool::worker_thread, this, 0, nullptr);

      int err = errno;
      if (err != 0 || newthread == nullptr)
      {
        // std::cout << "Thread creation using _beginthreadex failed with " << err << " error code." << std::endl;
        std::cerr << "Thread creation using _beginthreadex failed with " << err << " error code." << std::endl;
      }
      else
      {
        threads.push_back(newthread);
      }

      // DWORD dw = GetLastError();
      // std::cout << std::endl
      //           << dw << std::endl;

      // LPCTSTR lpszFunction = TEXT("_beginthreadex");
      // LPVOID lpMsgBuf;
      // LPVOID lpDisplayBuf;
      // DWORD dw = GetLastError();
      // std::cout << std::endl
      //           << dw << std::endl;

      // FormatMessage(
      //     FORMAT_MESSAGE_ALLOCATE_BUFFER |
      //         FORMAT_MESSAGE_FROM_SYSTEM |
      //         FORMAT_MESSAGE_IGNORE_INSERTS,
      //     NULL,
      //     dw,
      //     MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      //     (LPTSTR)&lpMsgBuf,
      //     0, NULL);

      // lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
      //                                   (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
      // StringCchPrintf((LPTSTR)lpDisplayBuf,
      //                 LocalSize(lpDisplayBuf) / sizeof(TCHAR),
      //                 TEXT("%s failed with error %d: %s"),
      //                 lpszFunction, dw, lpMsgBuf);
      // MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

      // LocalFree(lpMsgBuf);
      // LocalFree(lpDisplayBuf);
      // ExitProcess(dw);

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
      // std::cout << "All threads failed";
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