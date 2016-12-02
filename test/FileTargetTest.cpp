/**
 * @file FileTargetTest.cpp
 *
 * @date 2016-12-02
 * @author talesm
 */

#include "../src/FileTarget.hpp"

#include <fstream>

#include "catch.hpp"

using namespace std;

#ifndef TEST_TEMP_PREFIX
#define TEST_TEMP_PREFIX "./test"
#endif
#define TEST_FILE(test_str) TEST_TEMP_PREFIX "/" test_str

TEST_CASE("FileTarget Happy", "[target]") {
	auto path1 = TEST_FILE("test1.txt");
	{
		ofstream f { path1, ios_base::binary };
		f << "Hello World" << endl;
	}
	FileTarget target { path1 };

	SECTION("Read"){
		REQUIRE(target.view(12) == "Hello World\n");
	}

	SECTION("Tell"){
		REQUIRE(target.tell() == 0);
		target.view(12);
		REQUIRE(target.tell() == 12);
	}

	SECTION("go"){
		target.go(7);
		REQUIRE(target.tell() == 7);
		target.go(-2);
		REQUIRE(target.tell() == 5);
		target.go(4);
		REQUIRE(target.tell() == 9);
	}

	SECTION("Write"){
		REQUIRE_NOTHROW(target.write("Weird"));
		target.toStart();
		REQUIRE(target.view(12) == "Weird World\n");
		target.toEnd();
		target.go(-1);
		REQUIRE_NOTHROW(target.write("!!!"));
		target.toStart();
		REQUIRE(target.view(14) == "Weird World!!!");
	}
}

TEST_CASE("FileTarget with Invalid file name", "[target]") {
	REQUIRE_THROWS(FileTarget("/chewbacca"));
}

