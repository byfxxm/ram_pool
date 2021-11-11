#pragma once
#include "pool.h"

struct leak_info;

class rampool_imp
{
public:
	rampool_imp();
	~rampool_imp();
	static rampool_imp& instance_thread();
	void* malloc(size_t);
	void free(void*);
	void* realloc(void*, size_t);
	void destroy();
	void leak(leak_info*);
	size_t size(void*);
	void gc();
	void auto_gc(bool);

private:
	slot* __slot_cast(void*) const;

private:
	array<pool, POOLNUM> __pools;
	thread __auto_gc_thrd;
	atomic<bool> __is_auto_gc = false;
};