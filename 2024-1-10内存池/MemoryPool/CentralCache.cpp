#include "CentralCache.h"

CentralCache* CentralCache::this_item = nullptr;
std::recursive_mutex CentralCache::mutex;
  std::unordered_map<size_t, SpanNode*>CentralCache::PageIdMap;


CentralCache::CentralCache() {
	 
}


std::pair<void*, size_t> CentralCache::Allocate(size_t size, size_t index) {
	//先加锁
 
	//在这里我们是只需要加一个表锁就可以的
	std::unique_lock<std::recursive_mutex>lock(mutex);
	 
	if (memoryMap[index].empty() == true) {
		check();
		SpanNode* span = getSpanFromPageCache(size);
		check();
		//这个新的span还没有切割,在这里进行切割操作
		span->slice(size);
		//还要进行插入缓存操作
		//这个和Deallocate是对应的
		for (size_t i = span->getPageId(); i < span->getPageId() + span->getPageCount(); ++i) {
			PageIdMap.insert(std::make_pair(i, span));
		}
		span->setIsUse(true);
		static size_t num = 0;
		if (index == 1) {
			num++;
			std::cout << "num :" << num << std::endl;
			std::cout << "spanLis num :" << memoryMap[index].getSize() << std::endl;
			if (num == 79 || num==80) {
				int a = 10;
			}
		}
		check();
		//就是在这里出现的问题
		memoryMap[index].push_front(span);
		check();
	}
	//到了这个理就是有内存了
	//先计算我们要获得多少的内存块
	check();
	size_t num = getNum(numMap[std::this_thread::get_id()][index]);
	numMap[std::this_thread::get_id()][index] = num;
	std::pair<void*,size_t>p = memoryMap[index].getMemoryByNum(num);
	check();
 
	return p;
}

void CentralCache::Deallocate(void* memory, size_t index) {
	//先加锁
	std::unique_lock<std::recursive_mutex>lock(mutex);
	//所有的memory都是一个桶里面的
	//一个一个的释放
	void* m = memory;
	void* next = m;
	//std::cout << "CentralCache loop begin\n";
	check();
	size_t i = 0;
	while (m != nullptr) {
		if (i == 2) {
			int a = 10;
		}
		check();
		//先保存后继结点
		next = *(void**)m;
		size_t id = (size_t)m / (4 * 1024);
		SpanNode*span =  PageIdMap[id];
		//也就是说之前都没有出现过这样的情况
		if (span->isUse() == false) {
			int q1 = 0;
		}
		span->free(m);
		//也就是说span从PageCache里面出来的时候 memoryCount 是满的，但是use是true
		//当前我们释放回去了，memoryCount 是满的，use是false ,可以释放回去给pageCache
		if (span->isUse() == false) {
			//先释放span
			check();
			span = memoryMap[index].erase(span);
			check();
			//是不是要将当前span的PageIdMap里面的东西清理掉？
			//std::cout << "PageCache Deallocate begin\n";
			check();
			//应该先将之前的请空
			for (size_t i = span->getPageId(); i < span->getPageId() + span->getPageCount(); ++i) {
				//将之前的都清理干净。
				PageIdMap.erase(i);
			}
			PageCache::getPageCache().Deallocate(span);
			check();
			//std::cout << "PageCache Deallocate end\n";
		}
		//向后走
		m = next;
		check();
		i++;
	}
	check();
	//std::cout << "CentralCache loop end\n";

}

SpanNode* CentralCache::getSpanFromPageCache(size_t size) {
	//std::cout << "PageCache Allocate   Begin\n";
	SpanNode* span = PageCache::getPageCache().Allocate(size);
	//std::cout << "PageCache Allocate   Over\n";
	return span;
}

void CentralCache::check() {
	std::unique_lock < std::recursive_mutex>lock(mutex);
	for (size_t i = 0; i < 16 + 56 + 56 + 56 + 24; ++i) {
		memoryMap[i].check();
	}
}