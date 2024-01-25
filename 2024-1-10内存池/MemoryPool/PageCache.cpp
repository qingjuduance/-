
#include "PageCache.h"

std::recursive_mutex PageCache::mutex;
PageCache* PageCache::this_item = nullptr;
  std::unordered_map<size_t, SpanNode*>PageCache::PageIdMap;


//我们应该将一个页里面的全都弄好
SpanNode* PageCache::Allocate(size_t size) {
	//先加锁
	//std::cout << "PageCache lock  Begin\n";
	 
	std::unique_lock<std::recursive_mutex>lock(mutex);
	//std::cout << "PageCache lock  Over\n";
	//控制下标，至少是 1 也就是一个4 kb
	size_t index = (size + ((4 * 1024) - 1)) / (4 * 1024);
	//std::cout << "PageCache empty  begin\n";
	if (memoryMap[index].empty() == false) {
		//std::cout << "PageCache empty  over\n";
		//直接拿
		//std::cout << "PageCache get Memory  Begin\n";
		SpanNode* span = memoryMap[index].pop_front();
		//对于已经出去的我们也要进行操作
		//这里是一定要有的，因为后面是会有内存还回来的可能性
		PageIdMap.erase(span->getPageId());
		PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);
		//std::cout << "PageCache get Mmeory  Over\n";
		return span;
	}
	//std::cout << "PageCache empty  over\n";
	//std::cout << "PageCache for find   Begin\n";
	//向后面找
	for (size_t i = index + 1; i < 129; ++i) {
		if (memoryMap[i].empty() == false) {
			//先移除
			SpanNode* span = memoryMap[i].pop_front();
			PageIdMap.erase(span->getPageId());
			PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);

			//分配内存
			SpanNode* newSpan = new SpanNode();
			//std::cout << "PageCache sliceToSpan   Begin\n";
			span->sliceToSpan(newSpan,index);
			//std::cout << "PageCache sliceToSpan   Over\n";
			//设置内存位置
			memoryMap[index].push_front(newSpan);
			memoryMap[i - index].push_front(span);

			//设置page
			PageIdMap.insert(std::make_pair(span->getPageId(), span));
			PageIdMap.insert(std::make_pair(span->getPageId() + span->getPageCount() - 1, span));
			//这一段是不需要的，因为等会就直接拿回去了
			PageIdMap[newSpan->getPageId()] = newSpan;
			PageIdMap[newSpan->getPageId() + newSpan->getPageCount() - 1] = newSpan;
			return Allocate(size);
		}
	}
	//std::cout << "PageCache for find     Begin\n";

	//都没有

	void* memory = SystemAlloc(128);
	SpanNode*span = new SpanNode(memory, 128);
	memoryMap[128].push_front(span);
	beginPageMap[span->getPageId()] = true;
	//下面这一步我们也不需要做，因为在分的时候会做这个操作
	//PageIdMap[span->getPageId()] = span;
	//PageIdMap[span->getPageId() + span->getPageCount() - 1] = span;
	return Allocate(size);
}

void PageCache::Deallocate(SpanNode* span) {
	//先加锁
	std::unique_lock<std::recursive_mutex>lock(mutex);
	bool is = false;
	do {
		//判断一下左右两边有没有可以合并的
		size_t leftId = span->getPageId() - 1;
		size_t rightId = span->getPageCount() + span->getPageId();
		//每次刚开始循环就要值为false;
		//不然循环不会停止
		is = false;
		if (PageIdMap.count(leftId) != 0) {
			SpanNode* cur = PageIdMap[leftId];
			//必须要空闲的才能合并
			//和左边的合并，要保证，自己不是beginPage
			if (cur->isUse() == false && beginPageMap.count(span->getPageId()) == 0) {
				//要先删除
				memoryMap[cur->getPageCount()].erase(cur);
				//调整
				//删除span在PageIdMap 里面的缓存
				PageIdMap.erase(span->getPageId());
				PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);
				cur->merge(span);
				delete span;
				span = cur;
				is = true;
			}
		}
		if (PageIdMap.count(rightId) != 0) {
			//必须要空闲的才能合并
			SpanNode* cur = PageIdMap[rightId];
			//比我们大的一定要不是起始页号
			//和右边合并的时候要保证右边不是beginPage
			if (cur->isUse() == false && beginPageMap.count(cur->getPageId()) == 0) {
				//要先删除
				memoryMap[cur->getPageCount()].erase(cur);
				//调整
				//删除cur在PageIdMap 里面的缓存
				PageIdMap.erase(cur->getPageId());
				PageIdMap.erase(cur->getPageId() + cur->getPageCount() - 1);
				span->merge(cur);
				delete cur;
				is = true;
			}
		}
	} while (is);
	 
	//如果比我们大的是起始页号，我们就不能合并
	//首先我们要记录所有的128*4kb的数据的起始页号
	//就是我们判断一下，我们要合并的页号，是不是起始页号，是否
	//如果起始页号，我们就不合并
	  
	//最后判断一下span
	//大于等于就返回
	//就在这下面，很明显是不对的，我们每次申请出去的128*4kb，应该按照我们是怎么样申请出去的就怎么样还出去
	if (span->getPageCount() >= 128) {
		//应该先将之前的请空
		//删除span在PageIdMap 里面的缓存
		PageIdMap.erase(span->getPageId());
		PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);
		//释放掉
		SystemFree(span->getMemory());
		beginPageMap.erase(span->getPageId());
		//在我们删除这个span之前
		delete span;
		return;
	}
	//放入链表中
	//这个PageCount
	memoryMap[span->getPageCount()].push_front(span);
	//重新进行缓存操作
	PageIdMap.insert(std::make_pair(span->getPageId(), span));
	PageIdMap.insert(std::make_pair(span->getPageId() + span->getPageCount() - 1, span));
}