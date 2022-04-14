#include "stdafx.h"
#include "rampool_imp.h"
#include "Block.h"
#include "Slot.h"

RampoolImp::RampoolImp() {
	size_t size = 0;
	for_each(pools_.begin(), pools_.end(), [&](auto& it) {
		size += GRANULARITY;
		it.Initialize(size, this);
		});
}

RampoolImp::~RampoolImp() {
	AutoGc(false);
	Destroy();
}

RampoolImp* RampoolImp::Instance() {
	static RampoolImp inst;
	return &inst;
}

void RampoolImp::Destroy() {
	for (auto& pool : pools_)
		pool.Destroy();
}

void* RampoolImp::Malloc(size_t Size) {
	if (Size == 0 || Size > MAXSIZE)
		return nullptr;

	return pools_[POOLINDEX(Size)].Malloc(Size);
}

void RampoolImp::Free(void* p) {
	if (!p)
		return;

	pools_[POOLINDEX(SlotCast(p)->normalize_size)].Free(p);
}

void* RampoolImp::Realloc(void* p, size_t size) {
	if (!p)
		return Malloc(size);

	auto slot = SlotCast(p);
	auto p1 = Malloc(size);
	memmove(p1, p, min(slot->actual_size, size));
	Free(p);
	return p1;
}

void RampoolImp::Leak(leak_info_s* info) {
	if (!info)
		return;

	memset(info, 0, sizeof(leak_info_s));
	for (auto& po : pools_) {
		info->Count += po.Count();
		info->total_size += po.Count() * po.GetSize();
		info->total_actual_size += po.Total();
	}

	assert(info->total_actual_size <= info->total_size);
}

size_t RampoolImp::Size(void* p) {
	return SlotCast(p)->actual_size;
}

void RampoolImp::Gc() {
	for (auto& pool : pools_)
		pool.Gc();
}

void RampoolImp::AutoGc(bool b) {
	is_auto_gc_ = b;

	if (is_auto_gc_) {
		if (auto_gc_thread_.joinable())
			return;

		auto_gc_thread_ = std::thread([this]() {
			while (is_auto_gc_) {
				for (auto& po : pools_)
				{
					if (po.NeedGc())
						po.Gc();
				}

				std::this_thread::yield();
			}
			});
	} else {
		if (auto_gc_thread_.joinable())
			auto_gc_thread_.join();
	}
}

inline Slot* RampoolImp::SlotCast(void* p) const {
	auto slt = POINTER_TO_slot_s(p);

	if (slt->owner != this || slt->valid != Slot::Valid::kUsed)
		throw std::exception("invalid ptr");

	return slt;
}