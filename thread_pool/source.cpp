#include <iostream>
#include <thread>
#include "thread_pool.cpp"

using std::cout;
using std::endl;

void welcome()
{
  cout << "This is a program with thread_pool implementation" << endl;
}

int main()
{
  std::thread welcome_thread(welcome);
  if (welcome_thread.joinable())
    welcome_thread.join();

  thread_pool pool;
  size_t threads_cnt = pool.get_threads_count();
  cout << "Threads available: " << threads_cnt << endl;
}