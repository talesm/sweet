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
		REQUIRE(getFileContent(path1) == "Hello World");
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
		insert(target, "darkness?");
		REQUIRE(readAll(target) == "Oh, Hello...darkness? World??");
		target.go(-1);
		insert(target, " old friend");
		REQUIRE(readAll(target) == "Oh, Hello...darkness old friend? World??");
	}

	SECTION("erase"){
		target.erase(5);
		REQUIRE(readAll(target) == " World");
		target.go(1);
		target.erase(3);
		REQUIRE(readAll(target) == " ld");
		target.erase(2);
		REQUIRE(readAll(target) == " ");
	}

	SECTION("erase2"){
		target.go(+5);
		target.erase(1);
		REQUIRE(readAll(target) == "HelloWorld");
		target.go(-1);
		target.erase(1);
		REQUIRE(readAll(target) == "HellWorld");
		target.go(-1);
		target.erase(5);
		REQUIRE(readAll(target) == "Held");
		target.toEnd();
		insert(target, " with me!");
		REQUIRE(readAll(target) == "Held with me!");
		target.toStart();
		target.go(6);
		target.erase(3);
		REQUIRE(readAll(target) == "Held w me!");
		target.erase(4);
		REQUIRE(readAll(target) == "Held w");
		target.go(-2);
		target.erase(2);
		REQUIRE(readAll(target) == "Held");
	}

	SECTION("flush replaced"){
		replace(target, "Weird");
		REQUIRE(getFileContent(path1) == "Hello World");
		target.flush();
		REQUIRE(getFileContent(path1) == "Weird World");
		REQUIRE(readAll(target) == "Weird World");
	}

	SECTION("flush insertion"){
		insert(target, "Oh, ");
		REQUIRE(getFileContent(path1) == "Hello World");
		target.flush();
		REQUIRE(readAll(target) == "Oh, Hello World");
		REQUIRE(getFileContent(path1) == "Oh, Hello World");
		target.toEnd();
		insert(target, "!!!");
		target.flush();
		REQUIRE(readAll(target) == "Oh, Hello World!!!");
		REQUIRE(getFileContent(path1) == "Oh, Hello World!!!");
		target.go(-9);
		insert(target, "...");
		target.flush();
		REQUIRE(readAll(target) == "Oh, Hello... World!!!");
		REQUIRE(getFileContent(path1) == "Oh, Hello... World!!!");
	}

	SECTION("flush everything"){
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

