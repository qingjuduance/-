 

#ifndef __UTIL__H__
#define __UTIL__H__
#include <mutex>
#include <thread>
#include <unordered_map>
#include <iostream>
#include <Windows.h>
#include <assert.h>
 
void* SystemAlloc(size_t kpage);
void SystemFree(void* ptr);

size_t getSize(size_t size);
size_t getIndex(size_t size);

size_t getNum(size_t);
#endif