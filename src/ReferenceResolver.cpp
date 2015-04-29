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

inline void resolveFunctionReferences(OthFile &file, Function &function);
inline void parseCallList(OthFile &file, Function &function, vector<ParsedCall> &callList, vector<Call> &newCallList, stack<vector<Call>*> &blockStack);

static stack<Function*> callStack_res;

static void pairAndEliminateBasicConflicts(vector<pair<OthFile*,Function*>> &newList, uint32_t lineN, OthFile &local,
		ParsedCall &call, Call &newCall, vector<OthFile *> &resolvedFiles, vector<uint32_t> &resolvedIndices) {

	bool isCastable = false;
	Function *castf = NULL;

	for (unsigned int i = 0; i < resolvedFiles.size(); i++) {
		Function *f = &(resolvedFiles[i]->functionList[resolvedIndices[i]]);
		if (f->nInputs == call.inParams.size() &&
				f->nOutputs == call.outParams.size() &&
				f->confNodes.size() == call.confNodes.size()) { // Verify arg lengths

			// -->-->--> Resolve the potential match **important important** this is the recursive call <--<--<--
			resolveFunctionReferences(*resolvedFiles[i], *f);
			// *******

			bool valid = true;
			bool hasIncompat = false;
			for (unsigned int inp = 0; inp < call.inParams.size(); inp++) {
				uint8_t cv = f->r_inputs[inp].getCompatibilityValue(newCall.inputs[inp].datatype());
				if (cv <= DT_CASTABLE) {
					valid = false;
					if (cv == DT_INCOMPATIBLE) hasIncompat = true;
				}
			}
			if (!hasIncompat) {
				isCastable = true;
				castf = f;
			}
			if (valid) newList.push_back(make_pair(resolvedFiles[i], f));
		}
	}

	parse_validate(!newList.empty(), lineN, isCastable ?
			"Reference could not be resolved: " + call.callName + " (function '" + castf->toString() + "' is a close match but some parameters must be cast)" :
			"Reference could not be resolved: " + call.callName + "; inputs/outputs/configuration nodes don't match any known functions");
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
			if (self.inputs[i].name == name) return VarReference(name, &file, &function, &self, i, self.inputs[i].datatype(), true);
		}
	} else { // Pipes (inputs only)
		while (true) {
			parse_validate(!blockStack.empty(), lineN, "Variable or pipe reference could not be resolved: " + name);
			vector<Call> * callScope = blockStack.top(); blockStack.pop();
			for (int i = callScope->size() - 1; i >= 0; i--) {
				Call * currentCall = &(callScope->at(i));
				for (uint32_t j = 0; j < currentCall->outputs.size(); j++) {
					VarReference v = currentCall->outputs[j];
					if (v.name == name) {
						return VarReference(name, &file, &function, currentCall, j, v.datatype(), false);
					}
				}
			}
		}
	}
	parse_validate(false, lineN, "Variable or pipe reference could not be resolved: " + name);
	return VarReference();
}

static void setCallInputs(OthFile &file, Function &function, stack<vector<Call>*> &blockStack, Call &call, ParsedCall &oldCall) {
	for (unsigned int i = 0; i < oldCall.inParams.size(); i++) {
		VarReference v = resolveVarReference(true, oldCall.inParams[i], oldCall.lineN, file, function, call, blockStack);
		call.inputs.push_back(v);
		parse_validate(!(v.isOptional() && function.variable_defaults[i].empty()), call.lineN,
				"Optional input specified for non-optional parameter");
	}
}

static void defineConfNodes(OthFile &file, Function &function, stack<vector<Call>*> &blockStack, Call &call, ParsedCall &oldCall) {
	for (unsigned int i = 0; i < oldCall.confNodes.size(); i++) {
		vector<ParsedCall> node = oldCall.confNodes[i];
		vector<Call> newCallList;

		string word = "";
		bool isOneWord = node.size() == 1 && qualifiesAsKeyword_strict(node[0]);
		if (isOneWord) word = node[0].callName;

		uint8_t declaredMode = call.callReference->confNode_types[i];
		bool isReference = false;
		uint32_t refIndex;

		for (refIndex = 0; refIndex < function.confNodes.size(); refIndex++) {
			if (word == function.confNodes[i]) {
				parse_validate(function.confNode_types[i] == declaredMode, call.lineN, "ConfNode reference doesn't match declared type: " + word);
				isReference = true;
				break;
			}
		}

		if (!isReference) {
			if (declaredMode == CHAIN || declaredMode == SOUT_CHAIN) {
				parseCallList(file, function, node, newCallList, blockStack);
			} else {
				parse_validate(isOneWord, call.lineN, "Configuration node is not a valid " + string(cn_kw(declaredMode)) + " expression");
			}
		}

		ConfNode cn = ConfNode(isReference, declaredMode, newCallList);

		if (isReference) {
			cn.reference = VarReference(&file, &function, refIndex);
		} else if (call.callReference->confNode_types[i] == SOUT_CHAIN) {
			Call lastCall = newCallList[newCallList.size() - 1];
			parse_validate(lastCall.outputs.size() == 1, lastCall.lineN, "Last call in an SOUT_CHAIN must have one output");
			cn.type = lastCall.outputs[0].datatype(); // TODO this might be problematic (datatype, not satisfied datatype)
		} else if (declaredMode == DATATYPE) {
			cn.type = evaluateDatatypeWithoutAbstracts(word, call.lineN, "Invalid DATATYPE node: " + word);
		} else if (declaredMode == CONSTANT) {
			cn.reference = resolveVarReference(false, word, call.lineN, file, function, call, blockStack);
			parse_validate(cn.reference.isConstant(), call.lineN, "CONSTANT " + word + " does not reference a constant");
		}
		call.confNodes.push_back(cn);
	}
}

inline void parseCallList(OthFile &file, Function &function, vector<ParsedCall> &callList, vector<Call> &newCallList, stack<vector<Call>*> &blockStack) {
	for (ParsedCall &call : callList) {// TODO add built-in functions
		string name = call.callName;
		vector<OthFile *> resolvedFiles;
		vector<uint32_t> resolvedIndices;

		Call newCall;
		newCall.lineN = call.lineN;
		newCall.isBlockStart = call.isBlockStart; //TODO and we need to process a special case!!
		newCall.isBlockEnd = call.isBlockEnd; //TODO and we need to process another special case!!
		setCallInputs(file, function, blockStack, newCall, call);

		findPotentialMatches(name, call.lineN, file, resolvedFiles, resolvedIndices);

		vector<pair<OthFile*,Function*>> resolved;
		pairAndEliminateBasicConflicts(resolved, call.lineN, file, call, newCall, resolvedFiles, resolvedIndices);

		parse_validate(resolved.size() == 1, call.lineN, "Ambiguous reference (multiple matches) for call " + call.callName);
		newCall.callReference = resolved[0].second;

		defineConfNodes(file, function, blockStack, newCall, call);

		blockStack.top()->push_back(newCall);
	}
}

inline void resolveFunctionReferences(OthFile &file, Function &function) {
	if (function.resolved) return; // TODO also check if it's already in the call stack (recursion) and kill static recursion
	// Split up variables
	for (unsigned int i = 0; i < function.variables.size(); i++) {
		if (i < function.nInputs) {
			function.r_inputs.push_back(function.variable_types[i]);
		} else if (i < function.nInputs + function.nOutputs) {
			function.r_outputs.push_back(function.variable_types[i]);
		} else {
			function.r_aux.push_back(function.variable_types[i]);
		}
	}
	callStack_res.push(&function);

	vector<Call> newCallList;
	stack<vector<Call>*> blockStack;
	blockStack.push(&newCallList);

	parseCallList(file, function, function.callList, newCallList, blockStack);

	callStack_res.pop();
	function.resolved = true;
}


#endif
