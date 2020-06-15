// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#pragma once

#include <cassert>
#include <cstdint>

namespace simple {
namespace allocator {

uint8_t alignForwardAdjustment(void* address, uint8_t alignment);
uint8_t alignForwardAdjustment(uintptr_t address, uint8_t alignment);
uint8_t alignForwardAdjustmentWithHeader(uintptr_t address, uint8_t alignment, uint8_t headerSize);


inline uint8_t alignForwardAdjustment(void* address, uint8_t alignment) {
	auto mask = alignment - 1;
	uint8_t adjustmnet = alignment - (reinterpret_cast<uintptr_t>(address) & mask);
	if (adjustmnet == alignment) { return 0; }

	return adjustmnet;
}

inline uint8_t alignForwardAdjustment(uintptr_t address, uint8_t alignment) {
	auto mask = alignment - 1;
	uint8_t adjustmnet = alignment - (address & mask);
	if (adjustmnet == alignment) { return 0; }

	return adjustmnet;
}

inline uint8_t alignForwardAdjustmentWithHeader(uintptr_t address, uint8_t alignment, uint8_t headerSize) {
	assert(alignment > 0);
	assert(alignment == 1 || alignment % 2 == 0);

	uint8_t adjustment = alignForwardAdjustment(address, alignment);
	uint8_t neededSpace = headerSize;

	if (adjustment < neededSpace) {
		neededSpace -= adjustment;
		adjustment += alignment * (neededSpace / alignment);
		if (neededSpace % alignment > 0) { adjustment += alignment; }
	}

	return adjustment;
}

} // namespace allocator
} // namespace simple
