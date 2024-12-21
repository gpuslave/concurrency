#include <iostream>
#include <thread>
#include <fstream>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <string>
#include <chrono>

#include "thread_pool.h"

using std::cout;
using std::endl;
using std::string;

std::mutex g_cout_mut;

void st()
{
  cout << "This program demonstrates concurrent processing using a custom thread pool: " << endl;
  cout << "Generates random floating point numbers in range [0,1)" << endl;
  cout << "Processes numbers in parallel using available CPU threads" << endl;
  cout << "Writes results to file.txt with numbers transformed to 10000.xxx format, where xxx is the 0.xxx of the generated numbers" << endl;
  cout << "Uses threadsafe queue for work scheduling" << endl
       << endl;
}

void work(std::shared_ptr<std::vector<float>> arr, uint32_t start, uint32_t stop, int id,
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
  st();

  thread_pool pool(&g_cout_mut);
  uint32_t threads_cnt = pool.get_threads_count();
  cout << "Threads currently available on this device: " << threads_cnt << endl
       << endl;

  const char *FILE_NAME = "file.txt";
  std::ofstream file(FILE_NAME);
  file.clear();
  file.close();

  const uint32_t size = 50;
  std::vector<float> data(size);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0, 1.0);

  std::generate(data.begin(), data.end(), [&]()
                { return dis(gen); });

  cout << "Randomly generated numbers (amount:" << size << "):" << endl;
  for (auto elem : data)
    cout << elem << endl;

  uint32_t step = floor(size / threads_cnt);
  cout << endl
       << "Parallel processing will be devided by portions, each portion will be " << step << " numbers" << endl
       << endl;

  uint32_t current_step = 0;
  std::mutex mut;

  std::shared_ptr<std::vector<float>> data_ptr = std::make_shared<std::vector<float>>(data);
  for (uint32_t i = 0; i < threads_cnt; i++)
  {
    uint32_t prev_step = current_step;
    if (i == threads_cnt - 1)
    {
      current_step = size;
    }
    else
    {
      current_step += step;
    }

    {
      std::lock_guard<std::mutex> cout_lk(g_cout_mut);
      cout << "Numbers from " << prev_step << " through " << current_step - 1 << " were submitted to the pool of processing" << endl;
      // cout << "Waiting for the worker to pick up the data and begin processing" << endl;
    }

    pool.submit(std::bind(work, data_ptr, prev_step, current_step, i, &mut, FILE_NAME));
  }
  std::this_thread::sleep_for(std::chrono::seconds(2));

  cout << endl
       << "Data processing is done" << endl;
}

void work(std::shared_ptr<std::vector<float>> arr, uint32_t start, uint32_t stop, int id,
          std::mutex *mut, const char *file_name)
{
  {
    std::lock_guard<std::mutex> cout_lk(g_cout_mut);
    std::cout << "Data bulk: " << id << " is being processed by the worker" << endl;
  }

  // cpu-heavy arbitrary processing (repeated +1 addition)
  for (uint32_t i = start; i < stop; ++i)
  {
    for (uint32_t j = 0; j < 10000; ++j)
    {
      (*arr)[i] = (*arr)[i] + 1;
    }
  }

  std::lock_guard<std::mutex> fileMutex(*mut);
  std::ofstream file(file_name, std::ios::app);

  file << std::fixed;
  file.precision(3);

  file.seekp(start);
  for (uint32_t i = start; i < stop; i++)
  {
    file << (*arr)[i] << endl;
  }

  file.close();

  {
    std::lock_guard<std::mutex> cout_lk(g_cout_mut);
    std::cout << "Data bulk: " << id << "  -- done " << endl;
  }
}