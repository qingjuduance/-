

#ifndef __PAGECACHE__H__
#define __PAGECACHE__H__
#include "Util.h"
#include "SpanList.h"
class PageCache {
private:
	//表锁
	static std::recursive_mutex mutex;
	//单例对象
	static PageCache* this_item;
	
	//记录开始的页号
	std::unordered_map<size_t, bool>beginPageMap;
	//记录内存
	SpanList memoryMap[129];
public:
	//记录着page页号的表
	static std::unordered_map<size_t, SpanNode*>PageIdMap;


	static PageCache& getPageCache() {
		if (this_item == nullptr) {
			std::unique_lock<std::recursive_mutex>lock(mutex);
			if (this_item == nullptr) {
				this_item = new PageCache();
			}
		}
		return *this_item;
	}

	SpanNode* Allocate(size_t size);

	void Deallocate(SpanNode* span);
};



#endif