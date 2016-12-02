/**
 * @file FileTargetTest.cpp
 *
 * @date 2016-12-02
 * @author talesm
 */

#include "../src/FileTarget.hpp"

#include <fstream>

#include "catch.hpp"
#include "fileUtils.hpp"


inline std::string read(FileTarget &target, long count){
	std::string buffer;
	target.view(count, back_inserter(buffer));
	return buffer;
}

inline void write(FileTarget &target, std::string const &buffer){
	target.replace(buffer.begin(), buffer.end());
}

TEST_CASE("FileTarget Happy", "[target]") {
	auto path1 = TEST_FILE("test1.txt");
	populateFile(path1, "Hello World");

	FileTarget target { path1 };

	SECTION("Read"){
		REQUIRE(read(target, 12) == "Hello World\n");
	}

	SECTION("Tell"){
		REQUIRE(target.tell() == 0);
		read(target, 12);
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
		REQUIRE_NOTHROW(write(target, "Weird"));
		target.toStart();
		REQUIRE(read(target, 12) == "Weird World\n");
		target.toEnd();
		target.go(-1);
		REQUIRE_NOTHROW(write(target, "!!!"));
		target.toStart();
		REQUIRE(read(target, 14) == "Weird World!!!");
	}
}

TEST_CASE("FileTarget with Invalid file name", "[target]") {
	REQUIRE_THROWS(FileTarget("/chewbacca"));
}

