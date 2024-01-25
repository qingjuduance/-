#include "Util.h"


void* SystemAlloc(size_t kpage)
{
#ifdef _WIN32
	void* ptr = VirtualAlloc(0, kpage * (1 << 12),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#else
	// brk mmap等
#endif
	if (ptr == nullptr)
		throw std::bad_alloc();
	return ptr;
}
void SystemFree(void* ptr)
{
#ifdef _WIN32
	VirtualFree(ptr, 0, MEM_RELEASE);
	 
#else
	// sbrk unmmap等
#endif
}


 size_t _RoundUp(size_t bytes, size_t align)
{
	return (((bytes)+align - 1) & ~(align - 1));
}
	// 对齐大小计算
 size_t getSize(size_t bytes)
{
	if (bytes <= 128) {
		return _RoundUp(bytes, 8);
	}
	else if (bytes <= 1024) {
		return _RoundUp(bytes, 16);
	}
	else if (bytes <= 8 * 1024) {
		return _RoundUp(bytes, 128);
	}
	else if (bytes <= 64 * 1024) {
		return _RoundUp(bytes, 1024);
	}
	else if (bytes <= 256 * 1024) {
		return _RoundUp(bytes, 8 * 1024);
	}
		return -1;
}
size_t _Index(size_t bytes, size_t align_shift)
{
	return ((bytes + (1 << align_shift) - 1) >> align_shift) - 1;
}
	// 计算映射的哪一个自由链表桶
 size_t getIndex(size_t bytes)
{
		
	// 每个区间有多少个链
	static int group_array[4] = { 16, 56, 56, 56 };
	if (bytes <= 128) {
		return _Index(bytes, 3);
	}
	else if (bytes <= 1024) {
		return _Index(bytes - 128, 4) + group_array[0];
	}
	else if (bytes <= 8 * 1024) {
		return _Index(bytes - 1024, 7) + group_array[1] + group_array[0];
	}
	else if (bytes <= 64 * 1024) {
		return _Index(bytes - 8 * 1024, 10) + group_array[2] + group_array[1]
			+ group_array[0];
	}
	else if (bytes <= 256 * 1024) {
		return _Index(bytes - 64 * 1024, 13) + group_array[3] +
			group_array[2] + group_array[1] + group_array[0];
	}
	return -1;
}


size_t getNum(size_t size) {
	if (size == 0) {
		size = 1;
	}
	else if (size < 32) {
		size *= 2;
	}
	else if (size < 512) {
		size += 1;
	}
	return  size;
}