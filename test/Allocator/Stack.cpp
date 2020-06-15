// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#include "Allocator/Stack.h"

#include <catch2/catch.hpp>

#include <cstdint>

namespace simple {

TEST_CASE("StackAllocator", "[StackAllocator]") {
	struct A {
		A() = default;
		A(float x, float y, float z, float w) : array { x, y, z, w } {}

		float array[4];
	};

	struct B {
		B() = default;
		B(uint64_t x, uint64_t y, uint64_t z) : array { x, y, z } {}

		uint64_t array[3];
	};

	const size_t size = 1024;
	void* memory = std::malloc(size);

	StackAllocator sa(memory, size);
	REQUIRE(sa.getSize() == size);

	SECTION("create") {
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);

		auto* a0 = sa.create<A>(1.1f, 1.2f, 1.3f, 1.4f);
		REQUIRE(a0 != nullptr);

		// last   = 0
		// header = 16
		// data   = 16 [4 * sizeof(float)]
		// align  = 0
		// sum    = 32 [0 + 16 + 16 + 0]
		REQUIRE(sa.getUsedMemory() == 32);
		REQUIRE(sa.getNumAllocations() == 1);

		auto* b0 = sa.create<B>(10, 20, 30);
		REQUIRE(b0 != nullptr);

		// last   = 32
		// header = 16
		// data   = 24 [3 * sizeof(uint64_t)]
		// align  = 0
		// sum    = 72 [32 + 16 + 24 + 0]
		REQUIRE(sa.getUsedMemory() == 72);
		REQUIRE(sa.getNumAllocations() == 2);


		REQUIRE(a0->array[0] == 1.1f);
		REQUIRE(a0->array[1] == 1.2f);
		REQUIRE(a0->array[2] == 1.3f);
		REQUIRE(a0->array[3] == 1.4f);

		REQUIRE(b0->array[0] == 10);
		REQUIRE(b0->array[1] == 20);
		REQUIRE(b0->array[2] == 30);


		sa.remove(b0);

		// last   = 72
		// header = 16
		// data   = 24 [3 * sizeof(uint64_t)]
		// align  = 0
		// sum    = 32 [72 - 16 - 24 - 0]
		REQUIRE(sa.getUsedMemory() == 32);
		REQUIRE(sa.getNumAllocations() == 1);

		sa.remove(a0);

		// last   = 32
		// header = 16
		// data   = 16 [4 * sizeof(float)]
		// align  = 0
		// sum    = 0 [32 - 16 - 16 - 0]
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);
	}

	SECTION("createNoConstruct") {
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);

		auto* a0 = sa.createNoConstruct<A>();
		REQUIRE(a0 != nullptr);

		a0->array[0] = 1.1f;
		a0->array[1] = 1.2f;
		a0->array[2] = 1.3f;
		a0->array[3] = 1.4f;

		// last   = 0
		// header = 16
		// data   = 16 [4 * sizeof(float)]
		// align  = 0
		// sum    = 32 [0 + 16 + 16 + 0]
		REQUIRE(sa.getUsedMemory() == 32);
		REQUIRE(sa.getNumAllocations() == 1);

		auto* b0 = sa.createNoConstruct<B>();
		REQUIRE(b0 != nullptr);

		b0->array[0] = 10;
		b0->array[1] = 20;
		b0->array[2] = 30;

		// last   = 32
		// header = 16
		// data   = 24 [3 * sizeof(uint64_t)]
		// align  = 0
		// sum    = 72 [32 + 16 + 24 + 0]
		REQUIRE(sa.getUsedMemory() == 72);
		REQUIRE(sa.getNumAllocations() == 2);


		REQUIRE(a0->array[0] == 1.1f);
		REQUIRE(a0->array[1] == 1.2f);
		REQUIRE(a0->array[2] == 1.3f);
		REQUIRE(a0->array[3] == 1.4f);

		REQUIRE(b0->array[0] == 10);
		REQUIRE(b0->array[1] == 20);
		REQUIRE(b0->array[2] == 30);


		sa.removeNoDestruct(b0);

		// last   = 72
		// header = 16
		// data   = 24 [3 * sizeof(uint64_t)]
		// align  = 0
		// sum    = 32 [72 - 16 - 24 - 0]
		REQUIRE(sa.getUsedMemory() == 32);
		REQUIRE(sa.getNumAllocations() == 1);

		sa.removeNoDestruct(a0);

		// last   = 32
		// header = 16
		// data   = 16 [4 * sizeof(float)]
		// align  = 0
		// sum    = 0 [32 - 16 - 16 - 0]
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint8_t>()") {
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);

		auto* a0 = sa.createArrayNoConstruct<uint8_t>(10);
		REQUIRE(a0 != nullptr);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		// last   = 0
		// header = 16
		// size   = 4
		// data   = 10 [10 * sizeof(uint8_t)]
		// align  = 0
		// sum    = 30 [0 + 16 + 4 + 10 + 0]
		REQUIRE(sa.getUsedMemory() == 30);
		REQUIRE(sa.getNumAllocations() == 1);

		auto* a1 = sa.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last   = 30
		// header = 16
		// data   = 8 [sizeof(uint64_t)]
		// align  = 2 [30 % 16 != 0] -> 32
		// sum    = 56 [30 + 16 + 8 + 2]
		REQUIRE(sa.getUsedMemory() == 56);
		REQUIRE(sa.getNumAllocations() == 2);


		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);


		sa.removeNoDestruct(a1);

		// last   = 56
		// header = 16
		// data   = 8 [sizeof(uint64_t)]
		// align  = 2
		// sum    = [56 - 16 - 8 - 2]
		REQUIRE(sa.getUsedMemory() == 30);
		REQUIRE(sa.getNumAllocations() == 1);

		sa.removeArrayNoDestruct(a0);

		// last   = 30
		// header = 16
		// size   = 4
		// data   = 10 [10 * sizeof(uint8_t)]
		// align  = 0
		// sum    = 0 [30 - 16 - 4 - 10]
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint16_t>()") {
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);

		auto* a0 = sa.createArrayNoConstruct<uint16_t>(10);
		REQUIRE(a0 != nullptr);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		// last   = 0
		// header = 16
		// size   = 4
		// data   = 20 [10 * sizeof(uint16_t)]
		// align  = 0
		// sum    = 40 [0 + 16 + 4 + 20 + 0]
		REQUIRE(sa.getUsedMemory() == 40);
		REQUIRE(sa.getNumAllocations() == 1);

		auto* a1 = sa.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last   = 40
		// header = 16
		// data   = 8 [sizeof(uint64_t)]
		// align  = 0
		// sum    = 64 [40 + 16 + 8 + 0]
		REQUIRE(sa.getUsedMemory() == 64);
		REQUIRE(sa.getNumAllocations() == 2);


		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);


		sa.removeNoDestruct(a1);

		// last   = 64
		// header = 16
		// data   = 8 [sizeof(uint64_t)]
		// align  = 0
		// sum    = 40 [64 - 16 - 8 - 0]
		REQUIRE(sa.getUsedMemory() == 40);
		REQUIRE(sa.getNumAllocations() == 1);

		sa.removeArrayNoDestruct(a0);

		// last   = 40
		// header = 16
		// size   = 4
		// data   = 20 [10 * sizeof(uint16_t)]
		// align  = 0
		// sum    = 0 [30 - 16 - 4 - 10]
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint32_t>()") {
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);

		auto* a0 = sa.createArrayNoConstruct<uint32_t>(10);
		REQUIRE(a0 != nullptr);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		// last   = 0
		// header = 16
		// size   = 4
		// data   = 40 [10 * sizeof(uint32_t)]
		// align  = 0
		// sum    = 60 [0 + 16 + 4 + 40 + 0]
		REQUIRE(sa.getUsedMemory() == 60);
		REQUIRE(sa.getNumAllocations() == 1);

		auto* a1 = sa.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last   = 60
		// header = 16
		// data   = 8 [sizeof(uint64_t)]
		// align  = 4 [60 % 8 != 0] -> 64
		// sum    = 88 [60 + 16 + 8 + 4]
		REQUIRE(sa.getUsedMemory() == 88);
		REQUIRE(sa.getNumAllocations() == 2);


		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);


		sa.removeNoDestruct(a1);

		// last   = 88
		// header = 16
		// data   = 8 [sizeof(uint64_t)]
		// align  = 0
		// sum    = 60 [88 - 16 - 8 - 4]
		REQUIRE(sa.getUsedMemory() == 60);
		REQUIRE(sa.getNumAllocations() == 1);

		sa.removeArrayNoDestruct(a0);

		// last   = 60
		// header = 16
		// size   = 4
		// data   = 40 [10 * sizeof(uint32_t)]
		// align  = 0
		// sum    = 0 [60 - 16 - 4 - 40]
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint64_t>()") {
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);

		auto* a0 = sa.createArrayNoConstruct<uint64_t>(10);
		REQUIRE(a0 != nullptr);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		// last   = 0
		// header = 16
		// size   = 4
		// align  = 4 [20 % 8 != 0] -> 24
		// data   = 80 [10 * sizeof(uint64_t)]
		// sum    = 104 [0 + 16 + 4 + 4 + 80]
		REQUIRE(sa.getUsedMemory() == 104);
		REQUIRE(sa.getNumAllocations() == 1);

		auto* a1 = sa.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last   = 104
		// header = 16
		// align  = 0
		// data   = 8 [sizeof(uint64_t)]
		// sum    = 128 [100 + 16 + 8 + 4]
		REQUIRE(sa.getUsedMemory() == 128);
		REQUIRE(sa.getNumAllocations() == 2);


		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);


		sa.removeNoDestruct(a1);

		// last   = 128
		// header = 16
		// align  = 0
		// data   = 8 [sizeof(uint64_t)]
		// sum    = 104 [128 - 16 - 8 - 0]
		REQUIRE(sa.getUsedMemory() == 104);
		REQUIRE(sa.getNumAllocations() == 1);

		sa.removeArrayNoDestruct(a0);

		// last   = 104
		// header = 16
		// size   = 4
		// align  = 4
		// data   = 80 [10 * sizeof(uint64_t)]
		// sum    = 0 [104 - 16 - 4 - 4 - 80]
		REQUIRE(sa.getUsedMemory() == 0);
		REQUIRE(sa.getNumAllocations() == 0);
	}

	std::free(memory);
}

} // namespace simple
