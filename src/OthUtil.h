#ifndef OTHUTIL_H_
#define OTHUTIL_H_

#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <sstream> // for to_string
using namespace std;

// Forward Defs
class Function;
class Call;
struct ParsedCall;
class ConfNode;
class OthFile;
class VarReference;

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
inline string trim_end(string s) {
	string::size_type len = s.length();
	string::size_type start = 0;

	while ((start < len) && (s.at(len - 1) <= ' ')) {
		len--;
	}
	return ((start > 0) || (len < s.length())) ?
			s.substr(start, len - start) : s;
}

inline void parse_validate(bool condition, int32_t lineN, string problem) {
	if (!condition) {
		cerr << "Parsing Error: " << problem << " at line " << lineN;
		exit(1);
	}
}

inline void parse_validate(bool condition, int32_t lineN, string problem, int32_t supplimentaryIndex) {
	if (!condition) {
		cerr << "Parsing Error: " << problem << supplimentaryIndex << " at line " << lineN;
		exit(1);
	}
}

inline uint32_t binlog(uint32_t bits) {
	int log = 0;
	if( ( bits & 0xffff0000 ) != 0 ) {bits >>= 16; log = 16;}
	if( bits >= 256 ) {bits >>= 8; log += 8;}
	if( bits >= 16 ) {bits >>= 4; log += 4;}
	if( bits >= 4 ) {bits >>= 2; log += 2;}
	return log + ( bits >> 1 );
}

inline void verifyExpectedBit(uint32_t bitCompound, uint32_t bitToCheck, int32_t lineN, string problem, int32_t index) {
	bool good = ((bitCompound>>binlog(bitToCheck)) & 1) != 0; // Check if bit is set
	parse_validate(good, lineN, problem, index);
}

inline string extractFilename(string path) {
	string::size_type found = path.find_last_of("/");
	return path.substr(found+1);
}

inline string extractDirectory(string path) {
	string::size_type found = path.find_last_of("/");
	return path.substr(0, found+1);
}

inline string intToString(int i) {
	std::stringstream ss;
	ss << i;
	return ss.str();
}

inline string getLocalIDExpression(string prefix, uint32_t ID, uint32_t lineN) {
	std::stringstream ss;
	ss << prefix << "_" << ID << "_" << lineN;
	return ss.str();
}

#endif
