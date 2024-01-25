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
	//链接
 
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
		//前后链接
		prev->next = next;
		next->prev = prev;
		////调整自己的指向为指向自己
		pop->next = pop;
		pop->prev = pop;
		//在这里拦截
		if (size == 0) {
			int a = 10;
		}
		size--;
		return pop;
	}
	return nullptr;
}
 
SpanNode* SpanList::erase(SpanNode* span) {
	//确认当前的span在这个链表里面
	if (span != head) {
		SpanNode* next = span->next;
		SpanNode* prev = span->prev;
		next->prev = prev;
		prev->next = next;
		//调整自己的指向
		span->next = span;
		span->prev = span;
		//在这里拦截
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
			//我们要将这个移动到前面去
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
	//在这里我们进行内存的切分操作
	//进行这个操作的一定是刚从PageCache里面拿出来的
	//只有pageId 和pageCount，memory 是正确的
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
	//先判断有没有这么多的内存块
	size_t actualNum = num > memoryCount ? memoryCount : num;
	//如果是全都拿走
	if (actualNum == memoryCount) {
		void* m = memory;
		memory = nullptr;
		memoryCount = 0;
		return std::make_pair(m,actualNum);
	}
	//只拿一部分
	memoryCount -= actualNum;
	void* m = memory;
	void* next = m;
	for (size_t i = 0; i < actualNum - 1; ++i) {
		next = *(void**)next;
	}
	memory = *(void**)next;
	*(void**)next = nullptr;
	//我们在这里进行检查操作
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
	//这里是忘记了链接回去的操作
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
	//监测memoryCount 是否和实际的相等
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