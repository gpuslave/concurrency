#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <chrono>

#include "thread_pool.h"

using std::cout;
using std::endl;

std::mutex g_cout_mut;

void st()
{
  cout << "Custom thread_pool implementation using threadsafe queue" << endl;
}

void work(std::shared_ptr<std::vector<float>> arr, size_t start, size_t stop, int id,
          std::mutex *mut, const char *file_name);

/**
 * @file source.cpp
 * @brief Thread pool implementation demonstration using concurrent file operations
 * @author gpuslave
 *
 * This program demonstrates concurrent processing using a custom thread pool:
 * - Generates random floating point numbers in range [0,1)
 * - Processes numbers in parallel using available CPU threads
 * - Writes results to file.txt with numbers transformed to 10000.xxx format, where xxx is the 0.xxx of the generated numbers
 * - Uses threadsafe queue for work scheduling
 *
 * @note Numbers may be affected by floating point rounding
 *
 * Build instructions:
 * - Use provided Makefile: `make`
 * - Executable will be in /build folder (exe: thread_pool)
 */
int main()
{
  std::thread welcome_thread(st);
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
  // std::mutex cout_mut;

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

    {
      std::unique_lock<std::mutex> cout_lk(g_cout_mut);
      cout << prev_step << " --- " << current_step << endl;
    }

    pool.submit(std::bind(work, data_ptr, prev_step, current_step, i, &mut, FILE_NAME));
  }
  std::this_thread::sleep_for(std::chrono::seconds(2));
}

void work(std::shared_ptr<std::vector<float>> arr, size_t start, size_t stop, int id,
          std::mutex *mut, const char *file_name)
{
  {
    std::unique_lock<std::mutex> cout_lk(g_cout_mut);
    std::cout << "worker: " << id << " started to process data" << endl;
  }

  // cpu-heavy arbitrary processing (repeated +1 addition)
  for (size_t i = start; i < stop; ++i)
  {
    for (size_t j = 0; j < 10000; ++j)
    {
      (*arr)[i] = (*arr)[i] + 1;
    }
  }

  std::unique_lock<std::mutex> fileMutex(*mut);
  std::ofstream file(file_name, std::ios::app);

  file << std::fixed;
  file.precision(3);

  file.seekp(start);
  for (size_t i = start; i < stop; i++)
  {
    file << (*arr)[i] << endl;
  }

  file.close();
  // std::cout << "thread " << id << "closed file" << endl;
}