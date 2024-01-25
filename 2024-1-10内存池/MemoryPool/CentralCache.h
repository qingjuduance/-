

#ifndef __CENTRALCACHE__H__
#define __CENTRALCACHE__H__
#include "Util.h"
#include "PageCache.h"
class CentralCache {
private:
	static CentralCache* this_item ;
	static std::recursive_mutex mutex;

	

	SpanList memoryMap[16 + 56 + 56 + 56 + 24];
	std::unordered_map<std::thread::id, std::unordered_map<size_t, size_t>>numMap;
public:
	//PageIdMap
	static std::unordered_map<size_t, SpanNode*>PageIdMap;
	static CentralCache& getCentralCache() {
		if (this_item == nullptr) {
			std::unique_lock<std::recursive_mutex>lock(mutex);
			if (this_item == nullptr) {
				this_item = new CentralCache();
			}
		}
		return *this_item;
	}
	CentralCache();
	std::pair<void*, size_t> Allocate(size_t size, size_t index);
	void Deallocate(void* memory, size_t index);

	SpanNode* getSpanFromPageCache(size_t size);
	void check();
};

#endif