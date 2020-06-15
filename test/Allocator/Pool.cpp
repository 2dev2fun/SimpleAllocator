// Copyright (C) 2020 Maxim, 2dev2fun@gmail.com. All rights reserved.

#include "Allocator/Pool.h"

#include <catch2/catch.hpp>

#include <cstdint>

namespace simple {

TEST_CASE("PoolAllocator", "[PoolAllocator]") {
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

	const size_t numObjects = 2;

	PoolAllocator<A> pa(numObjects);
	PoolAllocator<B> pb(numObjects);

	SECTION("create") {
		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 2);

		auto* a0 = pa.create(0.1f, 0.2f, 0.3f, 0.4f);
		REQUIRE(a0 != nullptr);

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 1);

		REQUIRE(a0->array[0] == 0.1f);
		REQUIRE(a0->array[1] == 0.2f);
		REQUIRE(a0->array[2] == 0.3f);
		REQUIRE(a0->array[3] == 0.4f);

		auto* a1 = pa.create(1.1f, 1.2f, 1.3f, 1.4f);
		REQUIRE(a1 != nullptr);

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 0);

		REQUIRE(a1->array[0] == 1.1f);
		REQUIRE(a1->array[1] == 1.2f);
		REQUIRE(a1->array[2] == 1.3f);
		REQUIRE(a1->array[3] == 1.4f);

		pa.remove(a0);
		pa.remove(a1);

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 2);

		auto* a2 = pa.create(2.1f, 2.2f, 2.3f, 2.4f);
		REQUIRE(a2 != nullptr);

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 1);

		REQUIRE(a2->array[0] == 2.1f);
		REQUIRE(a2->array[1] == 2.2f);
		REQUIRE(a2->array[2] == 2.3f);
		REQUIRE(a2->array[3] == 2.4f);

		auto* a3 = pa.create(3.1f, 3.2f, 3.3f, 3.4f);
		REQUIRE(a3 != nullptr);

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 0);

		REQUIRE(a3->array[0] == 3.1f);
		REQUIRE(a3->array[1] == 3.2f);
		REQUIRE(a3->array[2] == 3.3f);
		REQUIRE(a3->array[3] == 3.4f);


		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 2);

		auto* b0 = pb.create(10, 20, 30);
		REQUIRE(b0 != nullptr);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 1);

		REQUIRE(b0->array[0] == 10);
		REQUIRE(b0->array[1] == 20);
		REQUIRE(b0->array[2] == 30);

		auto* b1 = pb.create(40, 50, 60);
		REQUIRE(b1 != nullptr);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 0);

		REQUIRE(b1->array[0] == 40);
		REQUIRE(b1->array[1] == 50);
		REQUIRE(b1->array[2] == 60);

		pb.remove(b0);
		pb.remove(b1);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 2);

		auto* b2 = pb.create(15, 25, 35);
		REQUIRE(b2 != nullptr);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 1);

		REQUIRE(b2->array[0] == 15);
		REQUIRE(b2->array[1] == 25);
		REQUIRE(b2->array[2] == 35);

		auto* b3 = pb.create(45, 55, 65);
		REQUIRE(b3 != nullptr);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 0);

		REQUIRE(b3->array[0] == 45);
		REQUIRE(b3->array[1] == 55);
		REQUIRE(b3->array[2] == 65);


		pa.clean();
		pb.clean();

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 2);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 2);
	}

	SECTION("createNoConstruct") {
		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 2);

		auto* a0 = pa.createNoConstruct();
		REQUIRE(a0 != nullptr);

		a0->array[0] = 0.1f;
		a0->array[1] = 0.2f;
		a0->array[2] = 0.3f;
		a0->array[3] = 0.4f;

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 1);

		REQUIRE(a0->array[0] == 0.1f);
		REQUIRE(a0->array[1] == 0.2f);
		REQUIRE(a0->array[2] == 0.3f);
		REQUIRE(a0->array[3] == 0.4f);

		auto* a1 = pa.createNoConstruct();
		REQUIRE(a1 != nullptr);

		a1->array[0] = 1.1f;
		a1->array[1] = 1.2f;
		a1->array[2] = 1.3f;
		a1->array[3] = 1.4f;

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 0);

		REQUIRE(a1->array[0] == 1.1f);
		REQUIRE(a1->array[1] == 1.2f);
		REQUIRE(a1->array[2] == 1.3f);
		REQUIRE(a1->array[3] == 1.4f);

		pa.removeNoDestruct(a0);
		pa.removeNoDestruct(a1);

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 2);

		auto* a2 = pa.createNoConstruct();
		REQUIRE(a2 != nullptr);

		a2->array[0] = 2.1f;
		a2->array[1] = 2.2f;
		a2->array[2] = 2.3f;
		a2->array[3] = 2.4f;

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 1);

		REQUIRE(a2->array[0] == 2.1f);
		REQUIRE(a2->array[1] == 2.2f);
		REQUIRE(a2->array[2] == 2.3f);
		REQUIRE(a2->array[3] == 2.4f);

		auto* a3 = pa.createNoConstruct();
		REQUIRE(a3 != nullptr);

		a3->array[0] = 3.1f;
		a3->array[1] = 3.2f;
		a3->array[2] = 3.3f;
		a3->array[3] = 3.4f;

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 0);

		REQUIRE(a3->array[0] == 3.1f);
		REQUIRE(a3->array[1] == 3.2f);
		REQUIRE(a3->array[2] == 3.3f);
		REQUIRE(a3->array[3] == 3.4f);


		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 2);

		auto* b0 = pb.createNoConstruct();
		REQUIRE(b0 != nullptr);

		b0->array[0] = 10;
		b0->array[1] = 20;
		b0->array[2] = 30;

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 1);

		REQUIRE(b0->array[0] == 10);
		REQUIRE(b0->array[1] == 20);
		REQUIRE(b0->array[2] == 30);

		auto* b1 = pb.createNoConstruct();
		REQUIRE(b1 != nullptr);

		b1->array[0] = 40;
		b1->array[1] = 50;
		b1->array[2] = 60;

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 0);

		REQUIRE(b1->array[0] == 40);
		REQUIRE(b1->array[1] == 50);
		REQUIRE(b1->array[2] == 60);

		pb.removeNoDestruct(b0);
		pb.removeNoDestruct(b1);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 2);

		auto* b2 = pb.createNoConstruct();
		REQUIRE(b2 != nullptr);

		b2->array[0] = 15;
		b2->array[1] = 25;
		b2->array[2] = 35;

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 1);

		REQUIRE(b2->array[0] == 15);
		REQUIRE(b2->array[1] == 25);
		REQUIRE(b2->array[2] == 35);

		auto* b3 = pb.createNoConstruct();
		REQUIRE(b3 != nullptr);

		b3->array[0] = 45;
		b3->array[1] = 55;
		b3->array[2] = 65;

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 0);

		REQUIRE(b3->array[0] == 45);
		REQUIRE(b3->array[1] == 55);
		REQUIRE(b3->array[2] == 65);


		pa.clean();
		pb.clean();

		REQUIRE(pa.getNumTotalObjects() == 2);
		REQUIRE(pa.getNumFreeObjects() == 2);

		REQUIRE(pb.getNumTotalObjects() == 2);
		REQUIRE(pb.getNumFreeObjects() == 2);
	}
}

} // namespace simple
