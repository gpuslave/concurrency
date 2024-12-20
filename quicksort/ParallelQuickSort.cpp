#include <iostream>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <future>
#include <atomic>
#include <vector>
#include <random>
#include <utility>
#include <list>
#include <iomanip>

using namespace std;
using Time = std::chrono::steady_clock::time_point;

// sequential quicksort without async
template <typename T>
std::list<T> seq_quicksort(std::list<T> input)
{
  if (input.empty())
    return input;

  std::list<T> res;
  res.splice(res.begin(), input, input.begin()); // grab partition element out of the array
  T const &pivot = *res.begin();

  auto divide_point = std::partition(input.begin(), input.end(),
                                     [&](T const &t)
                                     { return t < pivot; }); // reorders input so that all t < pivot
                                                             // elements are first, returns first t that is >= pivot
  std::list<T> lower_part;
  lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);

  auto new_lower(seq_quicksort(std::move(lower_part)));
  auto new_higher(seq_quicksort(std::move(input)));

  res.splice(res.begin(), new_lower);
  res.splice(res.end(), new_higher);

  return res;
}

// async quicksort
template <typename T>
std::list<T> async_quicksort(std::list<T> input)
{
  if (input.empty())
    return input;

  std::list<T> res;
  res.splice(res.begin(), input, input.begin()); // grab partition element out of the array
  T const &pivot = *res.begin();

  auto divide_point = std::partition(input.begin(), input.end(),
                                     [&](T const &t)
                                     { return t < pivot; }); // reorders input so that all t < pivot
                                                             // elements are first, returns first t that is >= pivot
  std::list<T> lower_part;
  lower_part.splice(lower_part.begin(), input, input.begin(), divide_point);

  // system will determine right amount of thread spawning
  std::future<std::list<T>> new_lower(std::async(&async_quicksort<T>, std::move(lower_part)));

  auto new_higher(async_quicksort(std::move(input)));

  res.splice(res.begin(), new_lower.get());
  res.splice(res.begin(), new_higher);

  return res;
}

// old implementation
class ParallelQuickSort
{
public:
  static void quicksort_async(int *arr, int l, int r)
  {
    if (l >= r)
      return;

    int p = partition(arr, l, r);

    future<void> f = async(launch::async, quicksort_async, arr, l, p - 1);

    quicksort_async(arr, p + 1, r);

    // future is not shared and is void, so no get will be executed
    // if (f.valid())
    // {
    //   f.wait();
    // }
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
  const size_t size_small = 100;
  vector<int> data(size);
  vector<int> data_small(size_small);

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, 10000);

  generate(data.begin(), data.end(), [&]()
           { return dis(gen); });
  generate(data_small.begin(), data_small.end(), [&]()
           { return dis(gen); });

  cout << endl;

  const size_t test_1_size = 3;

  vector<pair<long, long>> test_1(test_1_size);
  vector<pair<long, long>> test_2(test_1_size);

  for (size_t i = 0; i < test_1_size; ++i)
  {
    test_1[i] = benchmark_sorting_ret(data);
    test_2[i] = benchmark_sorting_ret(data_small);
  }

  size_t k = 0;
  long test_1_seq_mean(0), test_1_async_mean(0);
  long test_2_seq_mean(0), test_2_async_mean(0);

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

  cout << endl
       << "small test" << endl;

  for (auto test_pair : test_2)
  {
    k++;

    test_2_seq_mean += test_pair.first;
    test_2_async_mean += test_pair.second;

    cout << "Test" << setw(2) << k << " " << setw(3) << test_pair.first << " seq micro seconds " << setw(6) << test_pair.second << " async micro seconds " << "\n";
  }

  test_2_seq_mean /= test_1_size;
  test_2_async_mean /= test_1_size;

  cout << "seq mean " << test_2_seq_mean << " micro seconds" << " async mean " << test_2_async_mean << " micro seconds";

  // new implementation test
  cout << endl;
  std::list<int> test_list(20, 0);

  generate(test_list.begin(), test_list.end(), [&]()
           { return dis(gen); });

  test_list = std::move(seq_quicksort(test_list));

  for (auto &i : test_list)
    cout << i << " ";

  return 0;
}
