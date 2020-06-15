// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#include "Allocator/Linear.h"

#include <catch2/catch.hpp>

#include <cstdint>

namespace simple {

TEST_CASE("Allocator Linear", "[LinearAllocator]") {
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

	LinearAllocator la(memory, size);
	REQUIRE(la.getSize() == size);

	SECTION("create") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.create<A>(1.5f, 2.5f, 3.5f, 4.5f);
		REQUIRE(a0 != nullptr);

		// last = 0
		// data = 4 * 4
		// sum  = 16
		REQUIRE(la.getUsedMemory() == 16);
		REQUIRE(la.getNumAllocations() == 1);

		auto* b0 = la.create<B>(150, 250, 350);
		REQUIRE(b0 != nullptr);

		// last = 16
		// data = 3 * 8
		// sum  = 40
		REQUIRE(la.getUsedMemory() == 40);
		REQUIRE(la.getNumAllocations() == 2);

		REQUIRE(a0->array[0] == 1.5f);
		REQUIRE(a0->array[1] == 2.5f);
		REQUIRE(a0->array[2] == 3.5f);
		REQUIRE(a0->array[3] == 4.5f);

		REQUIRE(b0->array[0] == 150);
		REQUIRE(b0->array[1] == 250);
		REQUIRE(b0->array[2] == 350);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	SECTION("createNoConstruct") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.createNoConstruct<A>();
		REQUIRE(a0 != nullptr);

		// last = 0
		// data = 4 * 4
		// sum  = 16
		REQUIRE(la.getUsedMemory() == 16);
		REQUIRE(la.getNumAllocations() == 1);


		auto* b0 = la.createNoConstruct<B>();
		REQUIRE(b0 != nullptr);

		// last = 16
		// data = 3 * 8
		// sum  = 40
		REQUIRE(la.getUsedMemory() == 40);
		REQUIRE(la.getNumAllocations() == 2);

		a0->array[0] = 1.5f;
		a0->array[1] = 2.5f;
		a0->array[2] = 3.5f;
		a0->array[3] = 4.5f;

		b0->array[0] = 150;
		b0->array[1] = 250;
		b0->array[2] = 350;

		REQUIRE(a0->array[0] == 1.5f);
		REQUIRE(a0->array[1] == 2.5f);
		REQUIRE(a0->array[2] == 3.5f);
		REQUIRE(a0->array[3] == 4.5f);

		REQUIRE(b0->array[0] == 150);
		REQUIRE(b0->array[1] == 250);
		REQUIRE(b0->array[2] == 350);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	SECTION("createArray") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.createArray<A>(2, 1.5f, 2.5f, 3.5f, 4.5f);
		REQUIRE(a0 != nullptr);

		// last   = 0
		// header = 4
		// data   = 2 * 4 * 4
		// sum    = 36
		// align  = 48 [36 % 16 != 0 -> 32 + 16 = 48]
		REQUIRE(la.getUsedMemory() == 48);
		REQUIRE(la.getNumAllocations() == 1);

		auto* b0 = la.create<B>(150, 250, 350);
		REQUIRE(b0 != nullptr);

		// last = 48
		// data = 24 [3 * 8]
		// sum  = 72
		REQUIRE(la.getUsedMemory() == 72);
		REQUIRE(la.getNumAllocations() == 2);

		REQUIRE(a0[0].array[0] == 1.5f);
		REQUIRE(a0[0].array[1] == 2.5f);
		REQUIRE(a0[0].array[2] == 3.5f);
		REQUIRE(a0[0].array[3] == 4.5f);

		REQUIRE(a0[1].array[0] == 1.5f);
		REQUIRE(a0[1].array[1] == 2.5f);
		REQUIRE(a0[1].array[2] == 3.5f);
		REQUIRE(a0[1].array[3] == 4.5f);

		REQUIRE(b0->array[0] == 150);
		REQUIRE(b0->array[1] == 250);
		REQUIRE(b0->array[2] == 350);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint8_t>()") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.createArrayNoConstruct<uint8_t>(10);
		REQUIRE(a0 != nullptr);

		// last   = 0
		// header = 4
		// data   = 10
		// sum    = 14
		REQUIRE(la.getUsedMemory() == 14);
		REQUIRE(la.getNumAllocations() == 1);

		auto* a1 = la.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last              = 14
		// align {8} 16 - 14 = 2
		// data              = 8
		// sum               = 24
		REQUIRE(la.getUsedMemory() == 24);
		REQUIRE(la.getNumAllocations() == 2);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint16_t>()") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.createArrayNoConstruct<uint16_t>(10);
		REQUIRE(a0 != nullptr);

		// last   = 0
		// header = 4
		// data   = 20
		// sum    = 24
		REQUIRE(la.getUsedMemory() == 24);
		REQUIRE(la.getNumAllocations() == 1);

		auto* a1 = la.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last              = 24
		// align {8} 24 - 24 = 0
		// data              = 8
		// sum               = 32
		REQUIRE(la.getUsedMemory() == 32);
		REQUIRE(la.getNumAllocations() == 2);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint32_t>()") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.createArrayNoConstruct<uint32_t>(10);
		REQUIRE(a0 != nullptr);

		// last   = 0
		// header = 4
		// data   = 40
		// sum    = 44
		REQUIRE(la.getUsedMemory() == 44);
		REQUIRE(la.getNumAllocations() == 1);

		auto* a1 = la.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last              = 44
		// align {8} 48 - 44 = 4
		// data              = 8
		// sum               = 56
		REQUIRE(la.getUsedMemory() == 56);
		REQUIRE(la.getNumAllocations() == 2);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(*a1 == UINT64_MAX);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	SECTION("createArrayNoConstruct<uint64_t>()") {
		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);

		auto* a0 = la.createArrayNoConstruct<uint64_t>(10);
		REQUIRE(a0 != nullptr);

		// last   = 0
		// header = 8
		// data   = 80
		// sum    = 88
		REQUIRE(la.getUsedMemory() == 88);
		REQUIRE(la.getNumAllocations() == 1);

		auto* a1 = la.createNoConstruct<uint64_t>();
		REQUIRE(a1 != nullptr);

		// last              = 88
		// align {8} 88 - 88 = 0
		// data              = 8
		// sum               = 96
		REQUIRE(la.getUsedMemory() == 96);
		REQUIRE(la.getNumAllocations() == 2);

		for (size_t i = 0; i < 10; ++i) {
			a0[i] = 12 + i;
		}

		for (size_t i = 0; i < 10; ++i) {
			REQUIRE(a0[i] == 12 + i);
		}

		*a1 = UINT64_MAX;
		REQUIRE(la.getSize() == size);

		la.clean();

		REQUIRE(la.getUsedMemory() == 0);
		REQUIRE(la.getNumAllocations() == 0);
	}

	std::free(memory);
}

} // namespace simple
