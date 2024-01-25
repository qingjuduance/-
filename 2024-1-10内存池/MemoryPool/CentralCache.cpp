#include "CentralCache.h"

CentralCache* CentralCache::this_item = nullptr;
std::recursive_mutex CentralCache::mutex;
  std::unordered_map<size_t, SpanNode*>CentralCache::PageIdMap;


CentralCache::CentralCache() {
	 
}


std::pair<void*, size_t> CentralCache::Allocate(size_t size, size_t index) {
	//�ȼ���
 
	//������������ֻ��Ҫ��һ�������Ϳ��Ե�
	std::unique_lock<std::recursive_mutex>lock(mutex);
	 
	if (memoryMap[index].empty() == true) {
		check();
		SpanNode* span = getSpanFromPageCache(size);
		check();
		//����µ�span��û���и�,����������и����
		span->slice(size);
		//��Ҫ���в��뻺�����
		//�����Deallocate�Ƕ�Ӧ��
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
		//������������ֵ�����
		memoryMap[index].push_front(span);
		check();
	}
	//���������������ڴ���
	//�ȼ�������Ҫ��ö��ٵ��ڴ��
	check();
	size_t num = getNum(numMap[std::this_thread::get_id()][index]);
	numMap[std::this_thread::get_id()][index] = num;
	std::pair<void*,size_t>p = memoryMap[index].getMemoryByNum(num);
	check();
 
	return p;
}

void CentralCache::Deallocate(void* memory, size_t index) {
	//�ȼ���
	std::unique_lock<std::recursive_mutex>lock(mutex);
	//���е�memory����һ��Ͱ�����
	//һ��һ�����ͷ�
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
		//�ȱ����̽��
		next = *(void**)m;
		size_t id = (size_t)m / (4 * 1024);
		SpanNode*span =  PageIdMap[id];
		//Ҳ����˵֮ǰ��û�г��ֹ����������
		if (span->isUse() == false) {
			int q1 = 0;
		}
		span->free(m);
		//Ҳ����˵span��PageCache���������ʱ�� memoryCount �����ģ�����use��true
		//��ǰ�����ͷŻ�ȥ�ˣ�memoryCount �����ģ�use��false ,�����ͷŻ�ȥ��pageCache
		if (span->isUse() == false) {
			//���ͷ�span
			check();
			span = memoryMap[index].erase(span);
			check();
			//�ǲ���Ҫ����ǰspan��PageIdMap����Ķ����������
			//std::cout << "PageCache Deallocate begin\n";
			check();
			//Ӧ���Ƚ�֮ǰ�����
			for (size_t i = span->getPageId(); i < span->getPageId() + span->getPageCount(); ++i) {
				//��֮ǰ�Ķ�����ɾ���
				PageIdMap.erase(i);
			}
			PageCache::getPageCache().Deallocate(span);
			check();
			//std::cout << "PageCache Deallocate end\n";
		}
		//�����
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