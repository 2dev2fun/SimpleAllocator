// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#pragma once

#include "Allocator.h"

#include <cassert>
#include <cstdlib>
#include <cstdint>
#include <utility>

namespace simple {

template <typename T>
class PoolAllocator {
public:
	PoolAllocator(uint32_t numObjects);
	~PoolAllocator();

	void clean();

	uint32_t getNumTotalObjects() const;
	uint32_t getNumFreeObjects() const;

	template <typename... Args>
	T* create(Args&&... args);

	T* createNoConstruct();

	void remove(T* object);
	void removeNoDestruct(T* object);
private:
	void* allocate();
	void free(void* pointer);
	void freeListInit();

	void*  mMemory;
	void** mFreeList;

	uint32_t mNumTotalObjects;
	uint32_t mNumFreeObjects;
	uint8_t  mAdjustment;
};


template <typename T>
PoolAllocator<T>::PoolAllocator(uint32_t numObjects) : mFreeList(nullptr), mAdjustment(0) {
	assert(sizeof(T) >= sizeof(void*));

	mMemory = std::malloc(numObjects * sizeof(T));
	mAdjustment = allocator::alignForwardAdjustment(mMemory, alignof(T));
	mNumTotalObjects = mNumFreeObjects = numObjects;
	freeListInit();
}

template <typename T>
PoolAllocator<T>::~PoolAllocator() {
	std::free(mMemory);
}

template <typename T>
void PoolAllocator<T>::clean() {
	mNumFreeObjects = mNumTotalObjects;
	freeListInit();
}

template <typename T>
void* PoolAllocator<T>::allocate() {
	assert(mNumFreeObjects > 0);
	assert(mFreeList);

	void* pointer = mFreeList;
	mFreeList = reinterpret_cast<void**>(*mFreeList);

	--mNumFreeObjects;

	return pointer;
}

template <typename T>
void PoolAllocator<T>::free(void* pointer) {
	assert(pointer >= mMemory);
	assert(mNumFreeObjects < mNumTotalObjects);

	*(reinterpret_cast<void**>(pointer)) = mFreeList;
	mFreeList = reinterpret_cast<void**>(pointer);

	++mNumFreeObjects;
}

template <typename T>
void PoolAllocator<T>::freeListInit() {
	mFreeList = reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(mMemory) + mAdjustment);
	void** pointer = mFreeList;

	for (uint32_t i = 0; i < mNumTotalObjects - 1; ++i) {
		*pointer = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pointer) + sizeof(T));
		pointer = reinterpret_cast<void**>(*pointer);
	}
}

template <typename T>
uint32_t PoolAllocator<T>::getNumTotalObjects() const {
	return mNumTotalObjects;
}

template <typename T>
uint32_t PoolAllocator<T>::getNumFreeObjects() const {
	return mNumFreeObjects;
}

template <typename T>
template <typename... Args>
T* PoolAllocator<T>::create(Args&&... args) {
	return new (allocate()) T(std::forward<Args>(args)...);
}

template <typename T>
T* PoolAllocator<T>::createNoConstruct() {
	return reinterpret_cast<T*>(allocate());
}

template <typename T>
void PoolAllocator<T>::remove(T* object) {
	assert(object);
	object->~T();
	free(object);
}

template <typename T>
void PoolAllocator<T>::removeNoDestruct(T* object) {
	assert(object);
	free(object);
}

} // namespace simple
