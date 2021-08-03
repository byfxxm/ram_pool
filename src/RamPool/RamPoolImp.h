#pragma once
#include "Pool.h"

class CBlock;
class CSlot;
class CRamPoolImp
{
public:
	CRamPoolImp();
	~CRamPoolImp();
	void* Malloc(size_t);
	void Free(void*);
	void Destroy();
	size_t Leak();
	static CRamPoolImp* Instance();

private:
	CPool m_Pools[POOLNUM];
};