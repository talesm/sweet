/**
 * @file MemoryTargetTest.cpp
 *
 * @date 2016-12-02
 * @author talesm
 */

#include "../src/MemoryTarget.hpp"

#include "catch.hpp"
#include "fileUtils.hpp"

TEST_CASE("Memory Target Test", "[target]"){
	auto path1 = TEST_FILE("test1.txt");
	populateFile(path1, "Hello World");
}

