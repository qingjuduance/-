
#include "PageCache.h"

std::recursive_mutex PageCache::mutex;
PageCache* PageCache::this_item = nullptr;
  std::unordered_map<size_t, SpanNode*>PageCache::PageIdMap;


//����Ӧ�ý�һ��ҳ�����ȫ��Ū��
SpanNode* PageCache::Allocate(size_t size) {
	//�ȼ���
	//std::cout << "PageCache lock  Begin\n";
	 
	std::unique_lock<std::recursive_mutex>lock(mutex);
	//std::cout << "PageCache lock  Over\n";
	//�����±꣬������ 1 Ҳ����һ��4 kb
	size_t index = (size + ((4 * 1024) - 1)) / (4 * 1024);
	//std::cout << "PageCache empty  begin\n";
	if (memoryMap[index].empty() == false) {
		//std::cout << "PageCache empty  over\n";
		//ֱ����
		//std::cout << "PageCache get Memory  Begin\n";
		SpanNode* span = memoryMap[index].pop_front();
		//�����Ѿ���ȥ������ҲҪ���в���
		//������һ��Ҫ�еģ���Ϊ�����ǻ����ڴ滹�����Ŀ�����
		PageIdMap.erase(span->getPageId());
		PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);
		//std::cout << "PageCache get Mmeory  Over\n";
		return span;
	}
	//std::cout << "PageCache empty  over\n";
	//std::cout << "PageCache for find   Begin\n";
	//�������
	for (size_t i = index + 1; i < 129; ++i) {
		if (memoryMap[i].empty() == false) {
			//���Ƴ�
			SpanNode* span = memoryMap[i].pop_front();
			PageIdMap.erase(span->getPageId());
			PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);

			//�����ڴ�
			SpanNode* newSpan = new SpanNode();
			//std::cout << "PageCache sliceToSpan   Begin\n";
			span->sliceToSpan(newSpan,index);
			//std::cout << "PageCache sliceToSpan   Over\n";
			//�����ڴ�λ��
			memoryMap[index].push_front(newSpan);
			memoryMap[i - index].push_front(span);

			//����page
			PageIdMap.insert(std::make_pair(span->getPageId(), span));
			PageIdMap.insert(std::make_pair(span->getPageId() + span->getPageCount() - 1, span));
			//��һ���ǲ���Ҫ�ģ���Ϊ�Ȼ��ֱ���û�ȥ��
			PageIdMap[newSpan->getPageId()] = newSpan;
			PageIdMap[newSpan->getPageId() + newSpan->getPageCount() - 1] = newSpan;
			return Allocate(size);
		}
	}
	//std::cout << "PageCache for find     Begin\n";

	//��û��

	void* memory = SystemAlloc(128);
	SpanNode*span = new SpanNode(memory, 128);
	memoryMap[128].push_front(span);
	beginPageMap[span->getPageId()] = true;
	//������һ������Ҳ����Ҫ������Ϊ�ڷֵ�ʱ������������
	//PageIdMap[span->getPageId()] = span;
	//PageIdMap[span->getPageId() + span->getPageCount() - 1] = span;
	return Allocate(size);
}

void PageCache::Deallocate(SpanNode* span) {
	//�ȼ���
	std::unique_lock<std::recursive_mutex>lock(mutex);
	bool is = false;
	do {
		//�ж�һ������������û�п��Ժϲ���
		size_t leftId = span->getPageId() - 1;
		size_t rightId = span->getPageCount() + span->getPageId();
		//ÿ�θտ�ʼѭ����ҪֵΪfalse;
		//��Ȼѭ������ֹͣ
		is = false;
		if (PageIdMap.count(leftId) != 0) {
			SpanNode* cur = PageIdMap[leftId];
			//����Ҫ���еĲ��ܺϲ�
			//����ߵĺϲ���Ҫ��֤���Լ�����beginPage
			if (cur->isUse() == false && beginPageMap.count(span->getPageId()) == 0) {
				//Ҫ��ɾ��
				memoryMap[cur->getPageCount()].erase(cur);
				//����
				//ɾ��span��PageIdMap ����Ļ���
				PageIdMap.erase(span->getPageId());
				PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);
				cur->merge(span);
				delete span;
				span = cur;
				is = true;
			}
		}
		if (PageIdMap.count(rightId) != 0) {
			//����Ҫ���еĲ��ܺϲ�
			SpanNode* cur = PageIdMap[rightId];
			//�����Ǵ��һ��Ҫ������ʼҳ��
			//���ұߺϲ���ʱ��Ҫ��֤�ұ߲���beginPage
			if (cur->isUse() == false && beginPageMap.count(cur->getPageId()) == 0) {
				//Ҫ��ɾ��
				memoryMap[cur->getPageCount()].erase(cur);
				//����
				//ɾ��cur��PageIdMap ����Ļ���
				PageIdMap.erase(cur->getPageId());
				PageIdMap.erase(cur->getPageId() + cur->getPageCount() - 1);
				span->merge(cur);
				delete cur;
				is = true;
			}
		}
	} while (is);
	 
	//��������Ǵ������ʼҳ�ţ����ǾͲ��ܺϲ�
	//��������Ҫ��¼���е�128*4kb�����ݵ���ʼҳ��
	//���������ж�һ�£�����Ҫ�ϲ���ҳ�ţ��ǲ�����ʼҳ�ţ��Ƿ�
	//�����ʼҳ�ţ����ǾͲ��ϲ�
	  
	//����ж�һ��span
	//���ڵ��ھͷ���
	//���������棬�������ǲ��Եģ�����ÿ�������ȥ��128*4kb��Ӧ�ð�����������ô�������ȥ�ľ���ô������ȥ
	if (span->getPageCount() >= 128) {
		//Ӧ���Ƚ�֮ǰ�����
		//ɾ��span��PageIdMap ����Ļ���
		PageIdMap.erase(span->getPageId());
		PageIdMap.erase(span->getPageId() + span->getPageCount() - 1);
		//�ͷŵ�
		SystemFree(span->getMemory());
		beginPageMap.erase(span->getPageId());
		//������ɾ�����span֮ǰ
		delete span;
		return;
	}
	//����������
	//���PageCount
	memoryMap[span->getPageCount()].push_front(span);
	//���½��л������
	PageIdMap.insert(std::make_pair(span->getPageId(), span));
	PageIdMap.insert(std::make_pair(span->getPageId() + span->getPageCount() - 1, span));
}