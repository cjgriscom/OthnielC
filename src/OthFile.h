#ifndef OTHFILE_H_
#define OTHFILE_H_

#include <vector>
#include <utility>
#include <Function.h>
#include <string>
#include <OthUtil.h>

using namespace std;

class OthFile {
public:
	string path;

	vector<string> imports;
	vector<pair<string,string>> aliases;

	vector<string> variables;
	vector<string> variable_types;
	vector<string> variable_defaults; // No default should be set to "". A default value for an input indicates optional.

	vector<Function> functionList;
};

#endif
