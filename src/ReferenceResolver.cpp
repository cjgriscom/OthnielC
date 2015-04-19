#ifndef REFERENCERESOLVER_CPP_
#define REFERENCERESOLVER_CPP_

#include <OthUtil.h>
#include <OthFile.h>
#include <Keywords.h>
#include <Datatypes.h>
#include <Function.h>
#include <ParsedCall.h>
#include <string>
#include <stack>
#include <vector>
#include <iostream>
using namespace std;

static stack<Function*> callStack_res;

inline void resolveFunctionReferences(OthFile &file, Function &function) {
	callStack_res.push(&function);
	for (ParsedCall &call : function.callList) {
		string name = call.callName;
		OthFile * resolvedFile = &file;
		vector<uint32_t> resolvedIndices;
		bool foundImport = false, foundLocal = false; // TODO add built-in functions
		for (pair<string, pair<OthFile*,vector<uint32_t>>> p : file.function_imports) {
			if (name == p.first) {
				foundImport = true;
				resolvedFile = p.second.first;
				resolvedIndices = p.second.second;
			}
		}
		for (uint32_t i = 0; i < file.functionList.size(); i++) { // Load all local functions
			Function f = file.functionList[i];
			if (name == f.functionName) {
				parse_validate(!foundImport, f.lineN, "Local function '" + f.functionName + "' conflicts with an import in " + (*resolvedFile).path);
				foundLocal = true;
				resolvedIndices.push_back(i);
			}
		}
		parse_validate(foundImport || foundLocal, call.lineN, "Reference could not be resolved: " + call.callName);
		cout << "Found " << resolvedIndices.size() << " valid references. Name: " << resolvedFile->functionList[resolvedIndices[0]].functionName << endl;
	}
	callStack_res.pop();

}

inline void resolveVarReferences(OthFile &file, vector<OthFile> loadedFileList) {
	//TODO
}


#endif
