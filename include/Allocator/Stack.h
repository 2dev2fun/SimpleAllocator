// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#pragma once

#include "Allocator.h"

#include <cassert>
#include <cstdint>
#include <utility>

namespace simple {

class StackAllocator {
	struct Header {
		uintptr_t mPreviousAddress;
		uint8_t   mAdjustment;
	};
public:
	StackAllocator(void* start, uint32_t size);
	~StackAllocator();

	void clean();

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

	template <typename T>
	void remove(T* object);

	template <typename T>
	void removeNoDestruct(T* object);

	template <typename T>
	void removeArray(T* object);

	template <typename T>
	void removeArrayNoDestruct(T* object);

private:
	StackAllocator(StackAllocator&) = delete;
	StackAllocator(const StackAllocator&) = delete;

	StackAllocator& operator=(StackAllocator&) = delete;
	StackAllocator& operator=(const StackAllocator&) = delete;

	void* allocate(uint32_t size, uint8_t alignment);
	void free(void* pointer);

	uintptr_t mStart;
	uintptr_t mCurrentPosition;
	uintptr_t mPreviousPosition;

	uint32_t mSize;
	uint32_t mUsedMemory;
	uint32_t mNumAllocations;
};


inline StackAllocator::StackAllocator(void* start, uint32_t size)
		: mStart(reinterpret_cast<uintptr_t>(start))
		, mCurrentPosition(reinterpret_cast<uintptr_t>(start))
		, mPreviousPosition(0)
		, mSize(size)
		, mUsedMemory(0)
		, mNumAllocations(0) {
	assert(mSize > 0);
}

inline StackAllocator::~StackAllocator() {
	assert(mNumAllocations == 0 && mUsedMemory == 0);
}

inline void StackAllocator::clean() {
	mCurrentPosition  = mStart;
	mNumAllocations   = 0;
	mUsedMemory       = 0;
	mPreviousPosition = 0;
}

inline uint32_t StackAllocator::getSize() const {
	return mSize;
}

inline uint32_t StackAllocator::getUsedMemory() const {
	return mUsedMemory;
}

inline uint32_t StackAllocator::getNumAllocations() const {
	return mNumAllocations;
}

template <typename T, typename... Args>
T* StackAllocator::create(Args&&... args) {
	return new (allocate(sizeof(T), alignof(T))) T(std::forward<Args>(args)...);
}

template <typename T>
T* StackAllocator::createNoConstruct() {
	return reinterpret_cast<T*>(allocate(sizeof(T), alignof(T)));
}

template <typename T, typename... Args>
T* StackAllocator::createArray(uint32_t length, Args&&... args) {
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
T* StackAllocator::createArrayNoConstruct(uint32_t length) {
	assert(length != 0);

	uint8_t headerSize = sizeof(uint32_t) / sizeof(T);

	if (sizeof(uint32_t) % sizeof(T) > 0) { headerSize += 1; }

	T* pointer = reinterpret_cast<T*>(allocate(sizeof(T) * (length + headerSize), alignof(T))) + headerSize;

	*(reinterpret_cast<uint32_t*>(pointer) - 1) = length;

	return pointer;
}

template <typename T>
void StackAllocator::remove(T* object) {
	assert(object);
	object->~T();
	free(object);
}

template <typename T>
void StackAllocator::removeNoDestruct(T* object) {
	assert(object);
	free(object);
}

template <typename T>
void StackAllocator::removeArray(T* object) {
	assert(object);

	uint32_t length = *(reinterpret_cast<uint32_t*>(object) - 1);

	for (uint32_t i = 0; i < length; ++i) {
		object[i].~T();
	}

	uint8_t headerSize = sizeof(uint32_t) / sizeof(T);

	if (sizeof(uint32_t) % sizeof(T) > 0) { headerSize += 1; }

	free(object - headerSize);
}

template <typename T>
void StackAllocator::removeArrayNoDestruct(T* object) {
	assert(object);

	uint8_t headerSize = sizeof(uint32_t) / sizeof(T);

	if (sizeof(uint32_t) % sizeof(T) > 0) { headerSize += 1; }

	free(object - headerSize);
}

inline void* StackAllocator::allocate(uint32_t size, uint8_t alignment) {
	assert(size != 0);

	uint8_t adjustment = allocator::alignForwardAdjustmentWithHeader(mCurrentPosition, alignment, sizeof(Header));

	assert(mUsedMemory + adjustment + size <= mSize);

	auto alignedAddress = mCurrentPosition + adjustment;

	auto* header = reinterpret_cast<Header*>(alignedAddress - sizeof(Header));

	header->mAdjustment      = adjustment;
	header->mPreviousAddress = mPreviousPosition;

	mPreviousPosition = alignedAddress;
	mCurrentPosition  = alignedAddress + size;

	mUsedMemory += size + adjustment;

	++mNumAllocations;

	return reinterpret_cast<void*>(alignedAddress);
}

inline void StackAllocator::free(void* pointer) {
	auto position = reinterpret_cast<uintptr_t>(pointer);

	assert(position == mPreviousPosition);

	auto* header = reinterpret_cast<Header*>(position - sizeof(Header));

	mUsedMemory -= mCurrentPosition - position + header->mAdjustment;

	mCurrentPosition  = position - header->mAdjustment;
	mPreviousPosition = header->mPreviousAddress;

	--mNumAllocations;
}

} // namespace simple
