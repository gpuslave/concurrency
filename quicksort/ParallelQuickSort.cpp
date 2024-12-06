#include <iostream>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <future>
#include <atomic>
#include <vector>
#include <random>
#include <utility>
#include <iomanip>

using namespace std;
using Time = std::chrono::steady_clock::time_point;

class ParallelQuickSort
{
private:
  static constexpr int SEQUENTIAL_THRESHOLD = 1000;

public:
  static void quicksort_async(int *arr, int l, int r)
  {
    if (l >= r)
      return;

    int p = partition(arr, l, r);

    future<void> f;
    f = async(launch::async, quicksort_async, arr, l, p - 1);

    quicksort_async(arr, p + 1, r);

    if (f.valid())
    {
      f.wait();
    }
  }

  static void quicksort(int *arr, int l, int r)
  {
    if (l >= r)
      return;
    int p = partition(arr, l, r);
    quicksort(arr, l, p - 1);
    quicksort(arr, p + 1, r);
  }

private:
  static int partition(int *arr, int l, int r)
  {
    int pivot = arr[(l + r) / 2];
    int i = l - 1;
    int j = r + 1;

    while (true)
    {
      do
      {
        i++;
      } while (arr[i] < pivot);
      do
      {
        j--;
      } while (arr[j] > pivot);
      if (i >= j)
        return j;
      swap(arr[i], arr[j]);
    }
  }
};

void benchmark_sorting(vector<int> &data)
{
  vector<int> data_copy = data;

  auto start = chrono::steady_clock::now();
  ParallelQuickSort::quicksort(&data[0], 0, data.size() - 1);
  auto sequential_time = chrono::steady_clock::now() - start;

  start = chrono::steady_clock::now();
  ParallelQuickSort::quicksort_async(&data_copy[0], 0, data_copy.size() - 1);
  auto parallel_time = chrono::steady_clock::now() - start;

  long seq_time = chrono::duration_cast<chrono::microseconds>(sequential_time).count();
  long par_time = chrono::duration_cast<chrono::microseconds>(parallel_time).count();

  cout << "Sequential time: " << seq_time << " micro seconds\n";
  cout << "Parallel time: " << par_time << " micro seconds\n";
}

pair<long, long> benchmark_sorting_ret(vector<int> &data)
{
  vector<int> data_copy = data;

  auto start = chrono::steady_clock::now();
  ParallelQuickSort::quicksort(&data[0], 0, data.size() - 1);
  auto sequential_time = chrono::steady_clock::now() - start;

  start = chrono::steady_clock::now();
  ParallelQuickSort::quicksort_async(&data_copy[0], 0, data_copy.size() - 1);
  auto parallel_time = chrono::steady_clock::now() - start;

  long seq_time = chrono::duration_cast<chrono::microseconds>(sequential_time).count();
  long asy_time = chrono::duration_cast<chrono::microseconds>(parallel_time).count();

  return pair<long, long>(seq_time, asy_time);
}

int main()
{
  const size_t size = 10'000;
  vector<int> data(size);

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, 10000);

  generate(data.begin(), data.end(), [&]()
           { return dis(gen); });

  // cout << benchmark_sorting_ret(data).first << " micro seconds " << "\n"
  //      << benchmark_sorting_ret(data).second << " micro seconds ";

  cout << endl;

  const size_t test_1_size = 10;
  vector<pair<long, long>> test_1(test_1_size);
  for (size_t i = 0; i < 10; ++i)
    test_1[i] = benchmark_sorting_ret(data);

  size_t k = 0;
  long test_1_seq_mean(0), test_1_async_mean(0);

  for (auto test_pair : test_1)
  {
    k++;

    test_1_seq_mean += test_pair.first;
    test_1_async_mean += test_pair.second;

    cout << "Test" << setw(2) << k << " " << setw(3) << test_pair.first << " seq micro seconds " << setw(6) << test_pair.second << " async micro seconds " << "\n";
  }

  test_1_seq_mean /= test_1_size;
  test_1_async_mean /= test_1_size;

  cout << "seq mean " << test_1_seq_mean << " micro seconds" << " async mean " << test_1_async_mean << " micro seconds";
  return 0;
}
