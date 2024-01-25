#include "SpanList.h"




SpanList::SpanList() {
	head = new SpanNode();
	size = 0;
}
SpanList::~SpanList() {
	delete head;
}

size_t SpanList:: getSize() {
	return size;
}

void SpanList::push_front(SpanNode*span) {
	SpanNode* next = head->next;
	SpanNode* prev = head;
	//����
 
	prev->next = span;
	span->prev = prev;
	next->prev = span;
	span->next = next;
	size++;
 
}
SpanNode* SpanList::pop_front() {
	
	if (head->next != head) {
		SpanNode* pop = head->next;
		SpanNode* prev = pop->prev;
		SpanNode* next = pop->next;
		//ǰ������
		prev->next = next;
		next->prev = prev;
		////�����Լ���ָ��Ϊָ���Լ�
		pop->next = pop;
		pop->prev = pop;
		//����������
		if (size == 0) {
			int a = 10;
		}
		size--;
		return pop;
	}
	return nullptr;
}
 
SpanNode* SpanList::erase(SpanNode* span) {
	//ȷ�ϵ�ǰ��span�������������
	if (span != head) {
		SpanNode* next = span->next;
		SpanNode* prev = span->prev;
		next->prev = prev;
		prev->next = next;
		//�����Լ���ָ��
		span->next = span;
		span->prev = span;
		//����������
		if (size == 0) {
			int a = 10;
		}
		size--;
		return span;
	}
	return nullptr;
}


bool SpanList::empty() {
	SpanNode* cur = head->next;
	for (; cur != head; cur = cur->next) {
		if (cur->empty() == false) {
			//����Ҫ������ƶ���ǰ��ȥ
			//std::cout << "erase\n";
			//cur = erase(cur);
			//push_front(cur);
			return false;
		}
	}
	return true;
}

std::pair<void*, size_t>SpanList::getMemoryByNum(size_t num) {
	SpanNode* span = head->next;
	for (; span != head; span = span->next) {
		if (span->empty() == false) {
			return span->getMemoryByNum(num);
		}
	}
	return std::make_pair(nullptr, 0);
}


void SpanList::check() {
	size_t size = this->size;
	SpanNode* cur = head->next;
	size_t count = 0;
	for (; cur != head; cur = cur->next) {
		//
		cur->check();
		count++;
	}
	if (count != size) {
		int a = 10;
	}
}

////////////

SpanNode::SpanNode() {
	prev = this;
	next = this;
}

SpanNode::~SpanNode() {
	prev = nullptr;
	next = nullptr;
	pageId = 0;
	pageCount = 0;
	memory = nullptr;
	memorySize = 0;
	memoryCount = 0;

}

SpanNode::SpanNode(void* m, size_t pCount) {
	SpanNode();
	memory = m;
	this->pageCount = pCount;
	pageId = (size_t)memory / (4 * 1024);
}
size_t SpanNode::getPageId() {
	return this->pageId;
}
size_t SpanNode::getPageCount() {
	return this->pageCount;
}
bool SpanNode::empty() {

	return this->memory == nullptr;
}

void SpanNode::slice(size_t size) {
	//���������ǽ����ڴ���зֲ���
	//�������������һ���Ǹմ�PageCache�����ó�����
	//ֻ��pageId ��pageCount��memory ����ȷ��
	 memorySize = size;
	 memoryCount = pageCount * (4 * 1024) / size;
	 void* m = memory;
	 for (size_t i = 0; i < memoryCount - 1; ++i) {
		 *(void**)m = (char*)m + size;
		 m = (char*)m + size;
	 }
	 *(void**)m = nullptr;
	 check();
}
std::pair<void*, size_t>SpanNode::getMemoryByNum(size_t num) {
	//���ж���û����ô����ڴ��
	size_t actualNum = num > memoryCount ? memoryCount : num;
	//�����ȫ������
	if (actualNum == memoryCount) {
		void* m = memory;
		memory = nullptr;
		memoryCount = 0;
		return std::make_pair(m,actualNum);
	}
	//ֻ��һ����
	memoryCount -= actualNum;
	void* m = memory;
	void* next = m;
	for (size_t i = 0; i < actualNum - 1; ++i) {
		next = *(void**)next;
	}
	memory = *(void**)next;
	*(void**)next = nullptr;
	//������������м�����
	check();
	return std::make_pair(m, actualNum);
}
void SpanNode::sliceToSpan(SpanNode* span, size_t pCount) {
	span->pageCount = pCount;
	span->memory = this->memory;
	span->pageId = this->pageId;
	
	this->pageCount -= pCount;
	this->memory = (char*)this->memory + pCount * (4 * 1024);
	this->pageId = (size_t)this->memory / (4 * 1024);
}


bool SpanNode::isUse() {
	return this->use;
}

void SpanNode::setIsUse(bool is) {
	use = is;
}

size_t SpanNode::getMemorySize() {
	return memorySize;
}

void SpanNode::free(void* m) {
	*(void**)m  = this->memory;
	//���������������ӻ�ȥ�Ĳ���
	this->memory = m;
	this->memoryCount++;
	if (memoryCount == ((pageCount * (4 * 1024)) / memorySize)) {
		use = false;
	}
}



void SpanNode::merge(SpanNode* span) {
	this->pageCount += span->pageCount;
}

void* SpanNode::getMemory() {
	return memory;
}
void SpanNode::check() {
	//���memoryCount �Ƿ��ʵ�ʵ����
	size_t count = 0;
	void* m = memory;
	void* next = m;
	while (next != nullptr) {
		next = *(void**)next;
		count++;
	}
	if (count != memoryCount) {
		int a = 0;
	}
}