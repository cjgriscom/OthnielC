#ifndef OTHUTIL_H_
#define OTHUTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <iostream>
using namespace std;

// Inspired by Java's trim function
inline string trim(string s) {
	string::size_type len = s.length();
	string::size_type start = 0;

	while ((start < len) && (s.at(start) <= ' ')) {
		start++;
	}
	while ((start < len) && (s.at(len - 1) <= ' ')) {
		len--;
	}
	return ((start > 0) || (len < s.length())) ?
			s.substr(start, len - start) : s;
}

static void parse_validate(bool condition, int32_t lineN, string problem) {
	if (!condition) {
		cout << "Parsing Error: " << problem << " at line " << lineN;
		exit(1);
	}
}

static void parse_validate(bool condition, int32_t lineN, string problem, int32_t supplimentaryIndex) {
	if (!condition) {
		cout << "Parsing Error: " << problem << " " << supplimentaryIndex << " at line " << lineN;
		exit(1);
	}
}

static uint32_t binlog(uint32_t bits) {
	int log = 0;
	if( ( bits & 0xffff0000 ) != 0 ) {bits >>= 16; log = 16;}
	if( bits >= 256 ) {bits >>= 8; log += 8;}
	if( bits >= 16 ) {bits >>= 4; log += 4;}
	if( bits >= 4 ) {bits >>= 2; log += 2;}
	return log + ( bits >> 1 );
}

static void verifyExpectedBit(uint32_t bitCompound, uint32_t bitToCheck, int32_t lineN, string problem, int32_t index) {
	bool good = ((bitCompound>>binlog(bitToCheck)) & 1) != 0; // Check if bit is set
	parse_validate(good, lineN, problem, index);
}

#endif
