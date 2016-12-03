/**
 * @file MemoryTargetTest.cpp
 *
 * @date 2016-12-02
 * @author talesm
 */

#include "../src/MemoryTarget.hpp"

#include "catch.hpp"
#include "fileUtils.hpp"

inline std::string read(MemoryTarget &target, size_t count){
	std::string buffer;
	target.view(count, back_inserter(buffer));
	return buffer;
}

inline std::string readRange(MemoryTarget &target, size_t pos, size_t count){
	std::string buffer;
	target.viewRange(pos, count, back_inserter(buffer));
	return buffer;
}
inline std::string readAll(MemoryTarget &target){
	std::string buffer;
	target.viewAll(back_inserter(buffer));
	return buffer;
}

inline void replace(MemoryTarget &target, std::string const &v){
	target.replace(v.begin(), v.end());
}

inline void insert(MemoryTarget &target, std::string const &v){
	target.insert(v.begin(), v.end());
}

TEST_CASE("Memory Target Test", "[target]"){
	auto path1 = TEST_FILE("test1.txt");
	populateFile(path1, "Hello World");
	MemoryTarget target{path1};

	SECTION("view"){
		REQUIRE(read(target, 11) == "Hello World");
		REQUIRE(readRange(target, 0, 5) == "Hello");
		auto path2 = TEST_FILE("test2.txt");
		populateFile(path2, "Hi Mondo!");
		MemoryTarget target2{path2};
		REQUIRE(read(target2, 11) == "Hi Mondo!");
	}

	SECTION("replace"){
		replace(target, "Weird");
		REQUIRE(readAll(target) == "Weird World");
		replace(target, " Times");
		REQUIRE(readAll(target) == "Weird Times");
		replace(target, "!!!");
		REQUIRE(readAll(target) == "Weird Times!!!");
	}

	SECTION("insert"){
		insert(target, "Oh, ");
		REQUIRE(readAll(target) == "Oh, Hello World");
		target.toEnd();
		insert(target, "??");
		REQUIRE(readAll(target) == "Oh, Hello World??");
		target.go(-8);
		insert(target, "...");
		REQUIRE(readAll(target) == "Oh, Hello... World??");
	}

	SECTION("erase"){
		target.erase(5);
		REQUIRE(readAll(target) == " World");
		target.go(1);
		target.erase(5);
		REQUIRE(readAll(target) == " ");
	}

	SECTION("flush"){
		REQUIRE(readAll(target) == "Hello World");
		target.erase(5);
		insert(target, "Hi");
		target.go(+1);
		replace(target, "Weird");
		REQUIRE(readAll(target) == "Hi Weird");
		REQUIRE(getFileContent(path1) == "Hello World");
		target.flush();
		REQUIRE(getFileContent(path1) == "Hi Weird");
	}
}

