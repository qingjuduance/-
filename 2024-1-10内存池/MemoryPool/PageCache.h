

#ifndef __PAGECACHE__H__
#define __PAGECACHE__H__
#include "Util.h"
#include "SpanList.h"
class PageCache {
private:
	//����
	static std::recursive_mutex mutex;
	//��������
	static PageCache* this_item;
	
	//��¼��ʼ��ҳ��
	std::unordered_map<size_t, bool>beginPageMap;
	//��¼�ڴ�
	SpanList memoryMap[129];
public:
	//��¼��pageҳ�ŵı�
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