#ifndef REFERENCERESOLVER_CPP_
#define REFERENCERESOLVER_CPP_

#include <OthUtil.h>
#include <Keywords.h>
#include <Datatypes.h>
#include <ParsedCall.h>
#include <Function.h>
#include <OthFile.h>
#include <ConfigurationNode.h>
#include <Call.h>
#include <string>
#include <stack>
#include <vector>
#include <iostream>
using namespace std;

static stack<Function*> callStack_res;

static void pairAndEliminateBasicConflicts(vector<pair<OthFile*,Function*>> &newList, uint32_t lineN, OthFile &local, ParsedCall &call,
		vector<OthFile *> &resolvedFiles, vector<uint32_t> &resolvedIndices) {

	for (unsigned int i = 0; i < resolvedFiles.size(); i++) {
		Function *f = &(resolvedFiles[i]->functionList[resolvedIndices[i]]);
		if (f->nInputs == call.inParams.size() &&
				f->nOutputs == call.outParams.size() &&
				f->confNodes.size() == call.confNodes.size()) {
			newList.push_back(make_pair(resolvedFiles[i], f));
		}
	}

	parse_validate(!resolvedIndices.empty(), lineN, "Reference could not be resolved: " + call.callName + "; number of inputs and outputs don't match any known functions");
}

static void findPotentialMatches(string name, uint32_t lineN, OthFile &local,
		vector<OthFile *> &resolvedFiles, vector<uint32_t> &resolvedIndices) {
	for (pair<string, pair<OthFile*,vector<uint32_t>>> p : local.function_imports) {
		if (name == p.first) {
			resolvedFiles.push_back(p.second.first);
			resolvedIndices = p.second.second;
		}
	}
	for (uint32_t i = 0; i < local.functionList.size(); i++) { // Load all local functions
		Function f = local.functionList[i];
		if (name == f.functionName) {
			resolvedFiles.push_back(&local);
			resolvedIndices.push_back(i);
		}
	}
	parse_validate(!resolvedFiles.empty(), lineN, "Reference could not be resolved: " + name);
}

static VarReference resolveVarReference(bool isInput, string name, uint32_t lineN, OthFile &file, Function &function, Call &self, stack<vector<Call>*> blockStack) {
	if (name == "?" || name == "^") return VarReference(); // Garbage/? pipe

	// Constants (inputs only)
	if (file.constant_imports.find(name) == file.constant_imports.end() ) { // Search local constants
		for (unsigned int i = 0; i < file.constants.size(); i++) {
			if (name == file.constants[i]) {
				parse_validate(isInput, lineN, "Cannot output to a constant");
				return VarReference(name, true, &file, i);
			}
		}
	} else { // Search imports
		pair<OthFile*,uint32_t> p = file.constant_imports[name];
		parse_validate(isInput, lineN, "Cannot output to a constant");
		return VarReference(name, true, p.first, p.second);
	}

	// Variables
	if (file.variable_imports.find(name) == file.variable_imports.end() ) { // Search local variables
		for (unsigned int i = 0; i < file.variables.size(); i++) {
			if (name == file.variables[i]) {
				return VarReference(name, true, &file, i);
			}
		}
	} else { // Search imports
		pair<OthFile*,uint32_t> p = file.variable_imports[name];
		return VarReference(name, true, p.first, p.second);
	}

	// Function in/out/aux
	for (unsigned int i = 0; i < function.variables.size(); i++) {
		if (function.variables[i] == name) {
			return VarReference(name, &file, &function, i);
		}
	}

	// Self-reference (outputs only)
	if (!isInput) {
		for (unsigned int i = 0; i < self.inputs.size(); i++) {
			//TODO
		}
	} else { // Pipes (inputs only)
		while (true) {
			parse_validate(!blockStack.empty(), lineN, "Variable or pipe reference could not be resolved: " + name);
			vector<Call> * callScope = blockStack.top(); blockStack.pop();
			for (unsigned int i = 0; i < callScope->size(); i++) {
				Call * currentCall = &(callScope->at(i));
				for (uint32_t j = 0; j < currentCall->outputs.size(); j++) {
					VarReference v = currentCall->outputs[j];
					if (v.name == name) {
						return VarReference(name, &file, &function, currentCall, j, v.datatype());
					}
				}
			}
		}
	}
}

static void setCallInputs(OthFile &file, Function &function, stack<vector<Call>*> &blockStack, Call &call, ParsedCall &oldCall) {
	for (unsigned int i = 0; i < oldCall.inParams.size(); i++) {
		VarReference v = resolveVarReference(true, oldCall.inParams[i], oldCall.lineN, file, function, call, blockStack);
		call.inputs.push_back(v);
		// TODO validate optional pipes
	}
}

inline void resolveFunctionReferences(OthFile &file, Function &function) {
	callStack_res.push(&function);

	vector<Call> newCallList;
	stack<vector<Call>*> blockStack;
	blockStack.push(&newCallList);

	for (ParsedCall &call : function.callList) {// TODO add built-in functions
		string name = call.callName;
		vector<OthFile *> resolvedFiles;
		vector<uint32_t> resolvedIndices;

		Call newCall;
		newCall.lineN = call.lineN;
		newCall.isBlockStart = call.isBlockStart; //TODO and we need to process a special case!!
		newCall.isBlockEnd = call.isBlockEnd; //TODO and we need to process another special case!!
		setCallInputs(file, function, blockStack, newCall, call);

		findPotentialMatches(name, call.lineN, file, resolvedFiles, resolvedIndices);
		cout << call.lineN << "	Found " << resolvedIndices.size() << " valid references. Name: " << resolvedFiles[0]->functionList[resolvedIndices[0]].functionName << endl;

		vector<pair<OthFile*,Function*>> resolved;
		pairAndEliminateBasicConflicts(resolved, call.lineN, file, call, resolvedFiles, resolvedIndices);
		cout << call.lineN << "	  --> " << resolved.size() << " valid references." << endl;

		//parse_validate(!foundImport, f.lineN, "Local function '" + f.functionName + "' conflicts with an import in " + (*resolvedFile).path);
		//vector<>
		blockStack.top()->push_back(newCall);
	}
	callStack_res.pop();

}


#endif
