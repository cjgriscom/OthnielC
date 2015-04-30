#include <OthUtil.h>
#include <OthFile.h>
#include <Function.h>
#include <Call.h>
#include <CallParser.cpp>
#include <FunctionBuilder.cpp>
#include <PipeManager.cpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ReferenceResolver.cpp>
using namespace std;

void parseFile(OthFile &container, string fileName) {
	ifstream input(fileName.substr(1));
	parse_validate(input.good(), 0, "Error reading reference " + fileName);
	int index = 0;
	for (std::string line; getline( input, line );) {
		addLine(++index, line);
	}
	input.close();

	parse();

	container.path = fileName;
	assembleFile(container, finalCalls);
	validateAndReplaceDeclarationConstants(container);
	validatePipeAndFunctionNames(container);
	replaceForwardingCharsAndConstants(container);
	// Done parsing; move on to reference resolution
}

void parseAndResolveDependencies(
		OthFile &file, string filename,
		vector<OthFile> &loadedFileList,
		vector<string> &loadedFileNameList,
		string currentPath) {
	vector<string>::iterator beginning = loadedFileNameList.begin();
	vector<string>::iterator iter = find(beginning, loadedFileNameList.end(), currentPath + filename);
	if (iter != loadedFileNameList.end()) {
		// File already exists
		file = loadedFileList[iter-beginning];
		cout << "Already Loaded: " << filename << endl;
		return;
	} else {
		cout << "Loading: " << filename << endl;
		parseFile(file, currentPath + filename);
		loadedFileList.push_back(file);
		loadedFileNameList.push_back(currentPath + filename);
		uint32_t loadedFileIndex = loadedFileList.size()-1;

		vector<string> aliasKeys;
		vector<string> newImports;
		for (pair<string,string> aliasPair : file.aliases) {
			newImports.push_back(aliasPair.first);
			aliasKeys.push_back(aliasPair.second);
		}
		for (string name : file.imports) newImports.push_back(name);
		for (string importName : newImports) {
			OthFile import;
			importName = importName.substr(1, importName.size()-2);
			string name = extractFilename(importName);
			string method = "*";
			string::size_type colonPos = name.find_last_of(":");
			if (colonPos != name.npos) {
				method = name.substr(colonPos+1);
				name = name.substr(0, colonPos);
			}
			parseAndResolveDependencies(import, name, loadedFileList, loadedFileNameList, currentPath + extractDirectory(importName));
			bool found = false;
			for (unsigned int i = 0; i < import.functionList.size(); i++) {
				string name = import.functionList[i].functionName;
				if (method == "*" || import.functionList[i].functionName == method) {
					found = true;
					string key = name;
					if (i < aliasKeys.size()) key = aliasKeys[i];

					if (file.function_imports.find(key) == file.function_imports.end()) {
						// No entry; add one
						file.function_imports[key] = make_pair(&import, vector<uint32_t>());
					}
					file.function_imports[key].second.push_back(i);
				}
			}
			for (unsigned int i = 0; i < import.variables.size(); i++) {
				string name = import.variables[i];
				if (method == "*" || import.variables[i] == method) {
					found = true;
					string key = name;
					if (i < aliasKeys.size()) key = aliasKeys[i];
					file.variable_imports[key] = make_pair(&import, i);
				}
			}
			for (unsigned int i = 0; i < import.constants.size(); i++) {
				string name = import.constants[i];
				if (name[0] == '_') continue; // Ignore artificially added constants
				if (method == "*" || import.constants[i] == method) {
					found = true;
					string key = name;
					if (i < aliasKeys.size()) key = aliasKeys[i];
					file.constant_imports[key] = make_pair(&import, i);
				}
			}
			parse_validate(found, 0, "Could not find reference: " + importName);
		}
		loadedFileList[loadedFileIndex] = file; // Reset list item
	}
}

void printTest(Datatype func, Datatype call) {
	string val = "equal";
	if (func.getCompatibilityValue(call) == 1) val = "castable";
	if (func.getCompatibilityValue(call) == 2) val = "compatible";
	if (func.getCompatibilityValue(call) == 0) val = "incompatible";
	cout << func.asString() << " with " << call.asString() << ": " << val << endl;
}

void testTypes() {
	Datatype string = Datatype(STRING);
	Datatype numeric = Datatype(NUMERIC);
	Datatype integer = Datatype(INTEGER);
	Datatype anything = Datatype(ANYTHING);

	printTest(Datatype(integer,1), Datatype(numeric,1));
	printTest(Datatype(string,5), Datatype(string,1));
	printTest(Datatype(anything, 2), Datatype(Datatype(string, 1),2));
	printTest(Datatype(BOOL), numeric);
	printTest(numeric, string);
	printTest(numeric, Datatype(I32));
	printTest(numeric, integer);
	printTest(integer, numeric);
	printTest(Datatype(I64), Datatype(I16));
	printTest(Datatype(I64), integer);
	printTest(Datatype(F80), numeric);

	printTest(Datatype(U32), Datatype(I8));
	printTest(Datatype(U32), Datatype(U16));
	printTest(Datatype(U32), Datatype(U64));

	printTest(Datatype(I32), Datatype(U8));
	printTest(Datatype(I32), Datatype(U16));
	printTest(Datatype(I32), Datatype(U32));
	printTest(Datatype(F64), Datatype(INTEGER));

}

int main(int argc, char **argv) {
	vector<OthFile> loadedFileList;
	vector<string> loadedFileNameList;

	OthFile main;
	parseAndResolveDependencies(main, "test_var_reference.othsrc", loadedFileList, loadedFileNameList, "/");
	Function mainFunction;
	bool found = false;
	for (Function &f : main.functionList) {
		if (f.memoryMode == STATIC && f.nInputs == 0 && f.nOutputs ==0 && f.confNodes.empty() && f.functionName == "main") {
			found = true;
			mainFunction = f;
		}

	}
	parse_validate(found, 0, "Could not locate static []main[] in top-level file");

	//for (OthFile &file : loadedFileList) { // Begin resolving dependencies and final stages of parsing
		//resolveVarReferences(file, loadedFileList);
		//consolidateBlocks(file);
	//}

	// Recursive resolution functions
	resolveFunctionReferences(main, mainFunction);
	//setInlineAuxiliaries();

	for (OthFile &file : loadedFileList) {
		//renameVariablesAndFunctions(file);

		//addExplicitNumericCasts();
		cout << endl << "FILE: " << file.path << endl;
		testFB(file);
	}

	testTypes();

}
