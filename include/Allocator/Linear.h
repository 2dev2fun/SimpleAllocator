// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#pragma once

#include "Allocator.h"

#include <cassert>
#include <cstdint>
#include <utility>

namespace simple {

class LinearAllocator {
public:
	LinearAllocator(void* start, uint32_t size);
	~LinearAllocator();

	void clean();

	uintptr_t getCurrentPosition() const;
	void setCurrentPosition(uintptr_t position);

	uint32_t getSize() const;
	uint32_t getUsedMemory() const;
	uint32_t getNumAllocations() const;

	template <typename T, typename... Args>
	T* create(Args&&... args);

	template <typename T>
	T* createNoConstruct();

	template <typename T, typename... Args>
	T* createArray(uint32_t length, Args&&... args);

	template <typename T>
	T* createArrayNoConstruct(uint32_t length);
private:
	LinearAllocator(LinearAllocator&) = delete;
	LinearAllocator(const LinearAllocator&) = delete;

	LinearAllocator& operator=(LinearAllocator&) = delete;
	LinearAllocator& operator=(const LinearAllocator&) = delete;

	void* allocate(uint32_t size, uint8_t alignment);

	uintptr_t mStart;
	uintptr_t mCurrentPosition;

	uint32_t mSize;
	uint32_t mUsedMemory;
	uint32_t mNumAllocations;
};


inline LinearAllocator::LinearAllocator(void* start, uint32_t size)
		: mStart(reinterpret_cast<uintptr_t>(start))
		, mCurrentPosition(reinterpret_cast<uintptr_t>(start))
		, mSize(size)
		, mUsedMemory(0)
		, mNumAllocations(0) {
	assert(mSize > 0);
}

inline LinearAllocator::~LinearAllocator() {
	assert(mNumAllocations == 0 && mUsedMemory == 0);

	mStart           = 0;
	mCurrentPosition = 0;
	mSize            = 0;
}

inline void LinearAllocator::clean() {
	mNumAllocations  = 0;
	mUsedMemory      = 0;
	mCurrentPosition = mStart;
}

inline uintptr_t LinearAllocator::getCurrentPosition() const {
	return mCurrentPosition;
}

inline void LinearAllocator::setCurrentPosition(uintptr_t position) {
	assert(mCurrentPosition > position);
	assert(mStart < position);

	mCurrentPosition = position;
	mUsedMemory      = mCurrentPosition - mStart;
}

inline uint32_t LinearAllocator::getSize() const {
	return mSize;
}

inline uint32_t LinearAllocator::getUsedMemory() const {
	return mUsedMemory;
}

inline uint32_t LinearAllocator::getNumAllocations() const {
	return mNumAllocations;
}

template <typename T, typename... Args>
T* LinearAllocator::create(Args&&... args) {
	return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
}

template <typename T>
T* LinearAllocator::createNoConstruct() {
	return reinterpret_cast<T*>(allocate(sizeof(T), alignof(T)));
}

template <typename T, typename... Args>
T* LinearAllocator::createArray(uint32_t length, Args&&... args) {
	assert(length != 0);

	uint8_t headerSize = sizeof(uint32_t) / sizeof(T);

	if (sizeof(uint32_t) % sizeof(T) > 0) { headerSize += 1; }

	T* pointer = reinterpret_cast<T*>(allocate(sizeof(T) * (length + headerSize), alignof(T))) + headerSize;

	*(reinterpret_cast<uint32_t*>(pointer) - 1) = length;

	for (uint32_t i = 0; i < length; ++i) {
		new (&pointer[i]) T(std::forward<Args>(args)...);
	}

	return pointer;
}

template <typename T>
T* LinearAllocator::createArrayNoConstruct(uint32_t length) {
	assert(length != 0);

	uint8_t headerSize = sizeof(uint32_t) / sizeof(T);

	if (sizeof(uint32_t) % sizeof(T) > 0) { headerSize += 1; }

	T* pointer = reinterpret_cast<T*>(allocate(sizeof(T) * (length + headerSize), alignof(T))) + headerSize;

	*(reinterpret_cast<uint32_t*>(pointer) - 1) = length;

	return pointer;
}

inline void* LinearAllocator::allocate(uint32_t size, uint8_t alignment) {
	assert(size != 0);
	assert(alignment != 0);

	uint8_t adjustment = allocator::alignForwardAdjustment(mCurrentPosition, alignment);

	assert(mUsedMemory + adjustment + size <= mSize);

	uintptr_t alignedAddress = mCurrentPosition + adjustment;

	mCurrentPosition = alignedAddress + size;
	mUsedMemory += size + adjustment;

	++mNumAllocations;

	return reinterpret_cast<void*>(alignedAddress);
}

} // namespace simple
