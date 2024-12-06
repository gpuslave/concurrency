#include <iostream>
#include <thread>

void hello()
{
  std::cout << "Hello";
}

// int main is an initial thread
int main()
{
  std::cout << std::thread::hardware_concurrency() << "\n";
  std::thread t{hello};
  t.join();
}