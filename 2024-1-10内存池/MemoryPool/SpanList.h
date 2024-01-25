#ifndef __SPANLIST__H__
#define __SPANLIST_H__

#include "Util.h"

class SpanNode;

class SpanList {
private:
	SpanNode* head = nullptr;
	size_t size = 0;
public:
	SpanList();
	~SpanList();
	void push_front(SpanNode*span);
	SpanNode* pop_front();
	SpanNode* erase(SpanNode*span);
	bool empty();
	std::pair<void*, size_t>getMemoryByNum(size_t num);
	void check();
	size_t getSize();
};


class SpanNode {
private:
	friend SpanList;
	SpanNode* prev = nullptr;
	SpanNode* next = nullptr;

	void* memory = nullptr;
	size_t pageId = 0;
	size_t pageCount = 0;
	size_t memoryCount = 0;
	size_t memorySize = 0;
	bool use = false;
public:
	SpanNode();
	SpanNode(void*m,size_t pCount);
	~SpanNode();
	size_t getPageId();
	size_t getPageCount();
	size_t getMemorySize();
	bool isUse();
	void setIsUse(bool is);
	bool empty();
	void slice(size_t size);
	std::pair<void*, size_t>getMemoryByNum(size_t num);
	void sliceToSpan(SpanNode* span,size_t pCount);
	void free(void* m);
	
	void merge(SpanNode* span);

	void* getMemory();
	void check();
};

#endif