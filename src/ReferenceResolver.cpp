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

inline void resolveFunctionReferences(OthFile &file, Function &function, vector<OthFile> &loadedFileList, vector<string> &loadedFileNameList) {
	callStack_res.push(&function);
	for (ParsedCall &call : function.callList) {
		string name = call.callName;
		bool found = false;
		for (pair<string,string> p : file.aliases) {
			if (name == p.second) {
				found = true; name = p.first; // TODO filename manipulation... ugh
			}
		}
	}
	callStack_res.pop();

}

inline void resolveVarReferences(OthFile &file, vector<OthFile> loadedFileList) {
	//TODO
}


#endif
