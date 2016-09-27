#pragma once

#include <atomic>
#include <inttypes.h>

/// 64bit only
#define ALIGNMENT 8

template <class T>
class ObjectPool
{
public:

	enum
	{
		POOL_MAX_SIZE = 4096, ///< must be power of 2
		POOL_SIZE_MASK = POOL_MAX_SIZE - 1
	};

	/// memory pre- allocation is just optional :)
	static void PrepareAllocation()
	{
		for (int i = 0; i < POOL_MAX_SIZE; ++i)
			mPool[i] = _aligned_malloc(sizeof(T), ALIGNMENT);

		mTailPos.fetch_add(POOL_MAX_SIZE);
	}

	static void* operator new(size_t objSize)
	{
		uint64_t popPos = mHeadPos.fetch_add(1);

		void* popVal = std::atomic_exchange(&mPool[popPos & POOL_SIZE_MASK], (void*)nullptr);
		if (popVal != nullptr)
			return popVal;

		return _aligned_malloc(objSize, ALIGNMENT);
	}

	static void	operator delete(void* obj)
	{
		uint64_t insPos = mTailPos.fetch_add(1);

		void* prevVal = std::atomic_exchange(&mPool[insPos & POOL_SIZE_MASK], obj);

		if (prevVal != nullptr)
			_aligned_free(prevVal);
	}


private:

	static std::atomic<void*>	 mPool[POOL_MAX_SIZE];
	static std::atomic<uint64_t> mHeadPos;
	static std::atomic<uint64_t> mTailPos;

	static_assert((POOL_MAX_SIZE & POOL_SIZE_MASK) == 0x0, "pool's size must be power of 2");
};

template <class T>
std::atomic<void*> ObjectPool<T>::mPool[POOL_MAX_SIZE] = {};

template <class T>
std::atomic<uint64_t> ObjectPool<T>::mHeadPos(0);

template <class T>
std::atomic<uint64_t> ObjectPool<T>::mTailPos(0);
