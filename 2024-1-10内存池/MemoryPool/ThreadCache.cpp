#include "ThreadCache.h"



ThreadCache::ThreadCache() {
	for (size_t i = 0; i < 16 + 56 + 56 + 56 + 24; ++i) {
		memoryMap[i] = nullptr;
		sizeMap[i] = 0;
	}
}
ThreadCache::~ThreadCache() {
	for (size_t i = 0; i < 16 + 56 + 56 + 56 + 24; ++i) {
		//释放内存
		CentralCache::getCentralCache().Deallocate(memoryMap[i], i);
	}
}

void* ThreadCache::Allocate(size_t size) {

	//check();
	//std::cout << "ThreadCache Allocate  Begin \n";
	if (size == 0) {
		size = 1; 
	}
	if (size > 256 * 1024) {
		void*memory = SystemAlloc((size + (4 * 1024) - 1) / (4 * 1024));
		bigMap[memory] = true;
		return memory;
	}
	size = getSize(size);
	size_t index = getIndex(size);

	if (memoryMap[index] == nullptr) {
		check();
		std::pair<void*,size_t>p = getMemoryFromCentralCache(size, index);
		check();
		memoryMap[index] = p.first;
		sizeMap[index] = p.second;
		check();
	}
	check();
	void* memory = memoryMap[index];
	memoryMap[index] = *(void**)memory;
	sizeMap[index]--;
	check();
	//std::cout << "ThreadCache Allocate   Over\n";
	return memory;
}

void ThreadCache::Deallocate(void* memory) {
	//check();
	if (memory == nullptr) {
		assert(memory == nullptr);
		return;
	}
	//std::cout << "ThreadCache Deallocate   Begin\n";
	if (bigMap.count(memory) != 0) {
		bigMap.erase(memory);
		SystemFree(memory);
		//std::cout << "ThreadCache Deallocate   Over\n";
		return;
	}
	 //先获得页号，得到当前的这个页所在的span
	size_t id = (size_t)memory / (4 * 1024);
	SpanNode*span = CentralCache::PageIdMap[id];
	 //对于我们获取到的内存大小之后，我们还需要进行转化
	size_t size = span->getMemorySize();
	size_t index = getIndex(size);
	check();
	
	*(void**)memory = memoryMap[index];
	memoryMap[index] = memory;
	sizeMap[index]++;
	check();
	//释放内存
	if (sizeMap[index] > maxSize) {
		check();
		//也就是说在下面这段代码中出现了问题
		freeMemoryToCentralCache(index);
		
		check();
		int a = 10;
	}
	//std::cout << "ThreadCache Deallocate   Over\n";
}

void ThreadCache::freeMemoryToCentralCache(size_t index) {
	//先切出来一半的内存块
	//这里是不可能全释放的，因为只有sizeMap[index]里面的值大于maxSize，才会释放一半
	void* memory = memoryMap[index];
	size_t size = sizeMap[index]/2;
	void* next = memory;
	for (size_t i = 0; i < size - 1; ++i) {
		next = *(void**)next;
	}
	memoryMap[index] = *(void**)next;
	sizeMap[index] -= size;
	*(void**)next = nullptr;
	//std::cout << "CentralCache Deallocate   Begin\n";
	check();
	CentralCache::getCentralCache().Deallocate(memory,index);
	check();
	//std::cout << "CentralCache Deallocate   Over\n";
}

std::pair<void*,size_t>ThreadCache::getMemoryFromCentralCache(size_t size, size_t index) {
	//std::cout << "CentralCache Allocate   Begin\n";
	std::pair<void*,size_t> p = CentralCache::getCentralCache().Allocate(size, index);
	//std::cout << "CentralCache Allocate   Over\n";
	return p;
}

void ThreadCache::check() {
	this_check();
	CentralCache::getCentralCache().check();
}

void ThreadCache::this_check() {
	for (size_t i = 0; i < 208; ++i) {
		size_t count = 0;
		void* m = memoryMap[i];
		void* next = m;
		while (next != nullptr) {
			next = *(void**)next;
			count++;
		}
		if (count != sizeMap[i]) {
			int a = 0;
		}
	}
}