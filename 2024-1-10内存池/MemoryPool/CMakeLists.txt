

#先添加成为库
add_library(MemoryPool MemoryPool.cpp   "ThreadCache.cpp"   "CentralCache.cpp"  "PageCache.cpp"  "SpanList.cpp"  "Util.cpp")

#再添加头文件

target_include_directories(MemoryPool INTERFACE "${CMAKE_CURRENT_SOUREC_DIR}")
