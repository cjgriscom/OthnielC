#ifndef OTHFILE_H_
#define OTHFILE_H_

class OthFile;

#include <vector>
#include <utility>
#include <Function.h>
#include <string>
#include <map>
#include <OthUtil.h>
#include <ConstantParser.h>

using namespace std;

#define IMPORT_FUNCTION 0
#define IMPORT_VARIABLE 1
#define IMPORT_CONSTANT 2

class OthFile {
	int constantID = 0;
public:
	string path;

	vector<string> imports;
	vector<pair<string,string>> aliases;

	// Key: Alias or import name, Value: pair<file, index>
	map<string, pair<OthFile*,vector<uint32_t>>> function_imports;
	map<string, pair<OthFile*,uint32_t>> variable_imports;
	map<string, pair<OthFile*,uint32_t>> constant_imports;

	vector<string> variables;
	vector<Datatype> variable_types;
	vector<string> variable_defaults; // No default should be set to "". A default value for an input indicates optional.
	vector<uint32_t> variable_lines;

	vector<string> constants;
	vector<Datatype> constant_types;
	vector<string> constant_values; // No default should be set to "". A default value for an input indicates optional.
	vector<uint32_t> constant_lines;

	vector<Function> functionList;

	string getConstant(string &expression, int lineN) {
		if (expression == "true") return "_true";
		if (expression == "false") return "_false";
		for (unsigned int i = 0; i < constants.size(); i++) {
			if (constant_values[i] == expression) {
				return constants[i];
			}
		}
		string newName = getLocalIDExpression("_constant", constantID++, lineN);
		constants.push_back(newName);
		constant_types.push_back(getConstantType(expression, lineN));
		constant_values.push_back(expression);
		constant_lines.push_back(lineN);
		return newName;
	}
};

#endif
