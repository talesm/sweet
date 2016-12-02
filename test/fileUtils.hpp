/**
 * @file fileUtils.hpp
 *
 * @date 2016-12-02
 * @author talesm
 */

#ifndef TEST_FILEUTILS_HPP_
#define TEST_FILEUTILS_HPP_

#include <fstream>

using namespace std;

#ifndef TEST_TEMP_PREFIX
#define TEST_TEMP_PREFIX "./test"
#endif
#define TEST_FILE(test_str) TEST_TEMP_PREFIX "/" test_str

inline void populateFile(const char* path1, const char* text) {
	ofstream f { path1, ios_base::binary };
	f << text << endl;
}

#endif /* TEST_FILEUTILS_HPP_ */
