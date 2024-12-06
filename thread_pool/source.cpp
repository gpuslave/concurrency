#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <chrono>
#include "thread_pool.cpp"

using std::cout;
using std::endl;

void welcome()
{
  cout << "This is a program with thread_pool implementation" << endl;
}

void work(std::shared_ptr<std::vector<float>> arr, size_t start, size_t stop, int id,
          std::mutex *mut, const char *file_name);

int main()
{
  std::thread welcome_thread(welcome);
  if (welcome_thread.joinable())
    welcome_thread.join();

  thread_pool pool;
  size_t threads_cnt = pool.get_threads_count();
  cout << "Threads available: " << threads_cnt << endl;

  const char *FILE_NAME = "file.txt";
  std::ofstream file(FILE_NAME);
  file.clear();
  file.close();

  const size_t size = 50;
  std::vector<float> data(size);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0, 1.0);

  std::generate(data.begin(), data.end(), [&]()
                { return dis(gen); });

  for (auto elem : data)
    cout << elem << endl;

  size_t step = floor(size / threads_cnt);
  cout << "step: " << step << endl;

  size_t current_step = 0;
  std::mutex mut;

  std::shared_ptr<std::vector<float>> data_ptr = std::make_shared<std::vector<float>>(data);
  for (size_t i = 0; i < threads_cnt; i++)
  {
    size_t prev_step = current_step;
    if (i == threads_cnt - 1)
    {
      current_step = size;
    }
    else
    {
      current_step += step;
    }

    // auto task = [&]()
    // {
    //   work(data, prev_step, current_step, i, &mut, FILE_NAME);
    // };
    // std::functional<void()> task_fun = task;

    // pool.submit(std::bind(work, std::ref(data), std::ref(prev_step), std::ref(current_step), i, &mut, FILE_NAME));
    cout << prev_step << " --- " << current_step << endl;
    pool.submit(std::bind(work, data_ptr, prev_step, current_step, i, &mut, FILE_NAME));
  }
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void work(std::shared_ptr<std::vector<float>> arr, size_t start, size_t stop, int id,
          std::mutex *mut, const char *file_name)
{
  for (size_t i = start; i < stop; ++i)
  {
    for (size_t j = 0; j < 1000; ++j)
    {
      (*arr)[i] = (*arr)[i] + 1;
    }
  }

  std::unique_lock<std::mutex> fileMutex(*mut);
  std::ofstream file(file_name, std::ios::app);
  std::cout << "id: " << id << endl;

  file.seekp(start);
  for (size_t i = start; i < stop; i++)
  {
    file << (*arr)[i] << endl;
  }

  file.close();
  // std::cout << "thread " << id << "closed file" << endl;
}