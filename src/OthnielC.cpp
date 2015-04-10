#include <CallParser.cpp>
#include <FunctionBuilder.cpp>
#include <PipeManager.cpp>
#include <ReferenceResolver.cpp>
#include <OthFile.h>
#include <iostream>
#include <fstream>
#include <algorithm>
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
						//XXX cout << "Adding entry to " << file.path << ": " << key << file.function_imports.size() << endl;
						file.function_imports[key] = make_pair(import, vector<uint32_t>());
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
					file.variable_imports[key] = pair<OthFile,uint32_t>(import, i);
				}
			}
			for (unsigned int i = 0; i < import.constants.size(); i++) {
				string name = import.constants[i];
				if (method == "*" || import.constants[i] == method) {
					found = true;
					string key = name;
					if (i < aliasKeys.size()) key = aliasKeys[i];
					file.constant_imports[key] = pair<OthFile,uint32_t>(import, i);
				}
			}
			parse_validate(found, 0, "Could not find reference: " + importName);
		}
		loadedFileList[loadedFileIndex] = file; // Reset list item
	}
}

int main(int argc, char **argv) {
	vector<OthFile> loadedFileList;
	vector<string> loadedFileNameList;

	OthFile main;
	parseAndResolveDependencies(main, "test.othsrc", loadedFileList, loadedFileNameList, "/");
	Function mainFunction;
	bool found = false;
	for (Function &f : main.functionList) {
		if (f.memoryMode == STATIC && f.nInputs == 0 && f.nOutputs ==0 && f.functionName == "main") {
			found = true;
			mainFunction = f;
		}

	}
	parse_validate(found, 0, "Could not locate static []main[] in top-level file");

	for (OthFile &file : loadedFileList) { // Begin resolving dependencies and final stages of parsing
		resolveVarReferences(file, loadedFileList);
		consolidateBlocks(file);
	}

	// Recursive resolution functions
	//resolveFunctionReferences(file, mainFunction, loadedFileList, loadedFileNameList);
	//setInlineAuxiliaries();

	for (OthFile &file : loadedFileList) {
		//renameVariablesAndFunctions(file);

		//addExplicitNumericCasts();
		cout << endl << "FILE: " << file.path << endl;
		testFB(file);
	}

}
