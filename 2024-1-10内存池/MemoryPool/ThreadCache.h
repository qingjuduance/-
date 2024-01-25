

#ifndef __THREADCACHE__H__

#define __THREADCACHE__H__

#include "Util.h"
#include "CentralCache.h"

class ThreadCache {
private:
	void* memoryMap[16 + 56 + 56 + 56 + 24];
	size_t sizeMap[16 + 56 + 56 + 56 + 24];
	std::unordered_map<void*, bool>bigMap;
	size_t maxSize = 8;
public:
	ThreadCache();
	~ThreadCache();

	void* Allocate(size_t size);

	void Deallocate(void* memory);

	void  freeMemoryToCentralCache(size_t index);
	std::pair<void*, size_t>getMemoryFromCentralCache(size_t size, size_t index);
	void check();
	void this_check();
};
#endif