// 2024-1-10内存池.cpp: 定义应用程序的入口点。
//

#include "2024-1-10内存池.h"
#include "ThreadCache.h"
#include <vector>
using namespace std;


void BenchmarkMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	std::vector<std::thread> vthread(nworks);
	std::atomic<size_t> malloc_costtime = 0;
	std::atomic<size_t> free_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&, k]() {
			std::vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					v.push_back(malloc(16));
					//v.push_back(malloc((16 + i) % 8192 + 1));
				}
				size_t end1 = clock();
				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					free(v[i]);
				}
				size_t end2 = clock();
				v.clear();
				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}
	for (auto& t : vthread)
	{
		t.join();
	}


	std::cout << nworks << "个线程并发执行" << rounds 
		<< "轮次，每轮次malloc " << ntimes << "次: 花费：" << malloc_costtime << "ms\n";
	//std::printf("%u个线程并发执行%u轮次，每轮次malloc %u次: 花费：%u ms\n",
		//nworks, rounds, ntimes, malloc_costtime);
	std::cout << nworks << "个线程并发执行" << rounds
		<< "轮次，每轮次free " << ntimes << "次: 花费：" << free_costtime << "ms\n";
	/*printf("%u个线程并发执行%u轮次，每轮次free %u次: 花费：%u ms\n",
		nworks, rounds, ntimes, free_costtime);*/
	std::cout << nworks << "个线程并发执行malloc&free " << nworks * rounds * ntimes
		<<  "次: 花费：" << malloc_costtime + free_costtime << "ms\n";
	//printf("%u个线程并发malloc&free %u次，总计花费：%u ms\n",
	//	nworks, nworks * rounds * ntimes, malloc_costtime + free_costtime);
}
thread_local ThreadCache memoryPool;
// 单轮次申请释放次数 线程数 轮次
void BenchmarkConcurrentMalloc(size_t ntimes, size_t nworks, size_t rounds)
{
	
	std::vector<std::thread> vthread(nworks);
	std::atomic<size_t> malloc_costtime = 0;
	std::atomic<size_t> free_costtime = 0;
	for (size_t k = 0; k < nworks; ++k)
	{
		vthread[k] = std::thread([&]() {
			std::vector<void*> v;
			v.reserve(ntimes);
			for (size_t j = 0; j < rounds; ++j)
			{
				size_t begin1 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					if ((i == 7 && j == 2)) {
						int bb = 0;
					}
					v.push_back(memoryPool.Allocate(16));
					//v.push_back(memoryPool.Allocate((16 + i) % 8192 + 1));
				}
				size_t end1 = clock();
				size_t begin2 = clock();
				for (size_t i = 0; i < ntimes; i++)
				{
					if (i == 242 && j == 2) {
						int a = 10;
					}
					if (j == 1 && i == 9713) {
						int a = 01;
					}
					if (j == 1 && i == 9717) {
						//这里开始变成了负数
						int a = 1;
					}
					memoryPool.Deallocate(v[i]);
				}
				size_t end2 = clock();
				v.clear();
				malloc_costtime += (end1 - begin1);
				free_costtime += (end2 - begin2);
			}
			});
	}
	for (auto& t : vthread)
	{
		t.join();
	}

	std::cout << nworks << "个线程并发执行" << rounds
		<< "轮次，每轮次concurrent alloc " << ntimes << "次: 花费：" << malloc_costtime << "ms\n";
	//printf("%u个线程并发执行%u轮次，每轮次concurrent alloc %u次: 花费：%u ms\n",
	//	nworks, rounds, ntimes, malloc_costtime);
	std::cout << nworks << "个线程并发执行" << rounds
		<< "轮次，每轮次concurrent dealloc " << ntimes << "次: 花费：" << free_costtime << "ms\n";
	//printf("%u个线程并发执行%u轮次，每轮次concurrent dealloc %u次: 花费：%u ms\n",
	//	nworks, rounds, ntimes, free_costtime);
	std::cout << nworks << "个线程并发执行concurrent alloc&dealloc " << nworks * rounds * ntimes
		<< "次: 花费：" << malloc_costtime + free_costtime << "ms\n";
	//printf("%u个线程并发concurrent alloc&dealloc %u次，总计花费：%u ms\n",
	//	nworks, nworks * rounds * ntimes, malloc_costtime + free_costtime);
}

void testGetMemory() {
	std::cout << "test begin\n";
	this_thread::sleep_for(1s);
	 
	vector<void*>nums;
	for (int i = 0; i < 1000; ++i) {
		for (int j = 0; j < 1000; ++j) {
			std::cout << "Allocate   i: " << i << "  j: " << j << std::endl;
			nums.push_back(memoryPool.Allocate(i * j));
		}
	}
	for (int i = 0; i < 1000; ++i) {
		for (int j = 0; j < 1000; ++j) {
			std::cout << "Deallocate  i: " << i << "  j: " << j << std::endl;
			memoryPool.Deallocate(nums.back());
			nums.pop_back();
		}
	}
	this_thread::sleep_for(1s);
	std::cout << "test over\n";
}

int main()
{
	cout << "Hello CMake." << endl;
	fun(10);

	//void* p = SystemAlloc(128);
	//size_t page = (size_t)p /( 4 * 1024);
	//void* pp = (void*)(page * 4 * 1024);
	//testGetMemory();
	//return 1;
	std::cout << "测试开始。。。。。\n";
	this_thread::sleep_for(1s);
	size_t n = 10000;
		cout << "==========================================================" <<
		endl;
	BenchmarkConcurrentMalloc(n, 1, 10);
	cout << endl << endl;
	BenchmarkMalloc(n, 1, 10);
	cout << "==========================================================" <<
		endl;
	std::cout << "测试结束。。。。。\n";
	return 0;
}
