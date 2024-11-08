#include <iostream>
#include <ctime>
#include <chrono>
#include <algorithm>
#include <future>
#include <atomic>
#include <vector>
#include <random>

using namespace std;
using Time = std::chrono::steady_clock::time_point;

class ParallelQuickSort {
	private:
		static atomic<int> thread_pool;
		static constexpr int SEQUENTIAL_THRESHOLD = 1000;

	public:
		static void quicksort_async(int* arr, int l, int r) {
			if (l >= r) return;

			if (r - l < SEQUENTIAL_THRESHOLD) {
				quicksort(arr, l, r);
				return;
			}

			int p = partition(arr, l, r);

			future<void> f;
			if (thread_pool > 0) {
				--thread_pool;
				f = async(launch::async, quicksort_async, arr, l, p - 1);
			}
			else {
				quicksort_async(arr, l, p - 1);
			}

			quicksort_async(arr, p + 1, r);

			if (f.valid()) {
				f.wait();
				++thread_pool;
			}
		}

		static void quicksort(int* arr, int l, int r) {
			if (l >= r) return;
			int p = partition(arr, l, r);
			quicksort(arr, l, p - 1);
			quicksort(arr, p + 1, r);
		}

	private:
		static int partition(int* arr, int l, int r) {
			int pivot = arr[(l + r) / 2];
			int i = l - 1;
			int j = r + 1;

			while (true) {
				do { i++; } while (arr[i] < pivot);
				do { j--; } while (arr[j] > pivot);
				if (i >= j) return j;
				swap(arr[i], arr[j]);
			}
		}
};

atomic<int> ParallelQuickSort::thread_pool(8);

void benchmark_sorting(vector<int>& data) {
	vector<int> data_copy = data;

	auto start = chrono::steady_clock::now();
	ParallelQuickSort::quicksort(&data[0], 0, data.size() - 1);
	auto sequential_time = chrono::steady_clock::now() - start;

	start = chrono::steady_clock::now();
	ParallelQuickSort::quicksort_async(&data_copy[0], 0, data_copy.size() - 1);
	auto parallel_time = chrono::steady_clock::now() - start;

	cout << "Sequential time: " << chrono::duration_cast<chrono::microseconds>(sequential_time).count() << "µs\n";
	cout << "Parallel time: " << chrono::duration_cast<chrono::microseconds>(parallel_time).count() << "µs\n";
}

int main() {
	const int size = 10'000'000;
	vector<int> data(size);

	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dis(0, 10000);

	generate(data.begin(), data.end(), [&]() { return dis(gen); });

	benchmark_sorting(data);
	return 0;
}
