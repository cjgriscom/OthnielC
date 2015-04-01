#include <CallParser.cpp>
#include <FunctionBuilder.cpp>
#include <PipeManager.cpp>
#include <OthFile.h>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

void parseFile(OthFile &container, string fileName) {
	ifstream input(fileName.substr(1));

	int index = 0;
	for (std::string line; getline( input, line );) {
		addLine(++index, line);
	}
	input.close();

	parse();

	container.path = fileName;
	assembleFile(container, finalCalls);
	setPipeCodes(container);
}

void parseAndResolveDependencies(
		OthFile &file, string filename,
		vector<OthFile> &loadedFileList,
		vector<string> &loadedFileNameList,
		string currentPath) {
	if (find(loadedFileNameList.begin(), loadedFileNameList.end(), filename) != loadedFileNameList.end()) {
		// File already exists
		cout << "File exists: " << filename << endl;
		return;
	} else {
		cout << "Loading: " << filename << endl;
		parseFile(file, currentPath + filename);
		loadedFileList.push_back(file);
		loadedFileNameList.push_back(currentPath + filename);

		for (string importName : file.imports) {
			OthFile import;
			importName = importName.substr(1, importName.size()-2);
			string name = extractFilename(importName);
			int colonPos = name.find_last_of(":");
			if (colonPos != name.npos) name = name.substr(0, colonPos);
			parseAndResolveDependencies(import, extractFilename(importName) + ".othsrc", loadedFileList, loadedFileNameList, currentPath + extractDirectory(importName));
			file.imports_resolved.push_back(import);
		}
	}
}

int main(int argc, char **argv) {
	vector<OthFile> loadedFileList;
	vector<string> loadedFileNameList;

	OthFile main;
	parseAndResolveDependencies(main, "test.othsrc", loadedFileList, loadedFileNameList, "/");
	testFB(&main);
}
