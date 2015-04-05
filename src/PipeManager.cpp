#ifndef PIPEMANAGER_CPP_
#define PIPEMANAGER_CPP_

#include <queue>
#include <vector>
#include <string>
#include <OthUtil.h>
#include <OthFile.h>
#include <Keywords.h>
#include <Function.h>
#include <ParsedCall.h>
#include <ConstantParser.h>

using namespace std;

static int replaceForwardingCharactersAndConstants(OthFile &file, vector<ParsedCall> &calls, int startID) { //TODO I think this might have an issue with blocks....
	queue<string> forwardedPipes;
	int replacementID = startID;
	ParsedCall call;
	int lastForwardIndex = 0;
	for (unsigned int callN = 0; callN < calls.size(); callN++) {
		call = calls[callN];
		for (string &param : call.inParams) {
			parse_validate(param != ">", call.lineN, "Encountered > pipe in inputs");
			if (param == "<") {
				parse_validate(!forwardedPipes.empty(), call.lineN, "Encountered < but no forwarded pipes exist!");
				param = forwardedPipes.front();
				forwardedPipes.pop();
			} else if (isConstant(param)) {
				param = file.getConstant(param, call.lineN);
			}
		}
		for (string &param : call.outParams) {
			parse_validate(param != "<", call.lineN, "Encountered < pipe in outputs");
			if (param == ">") {
				param = getLocalIDExpression("_fwd", replacementID++, call.lineN);
				forwardedPipes.push(param);
				lastForwardIndex = call.lineN; //for error messages if needed
			}
			parse_validate(!isConstant(param), call.lineN, "Constants not allowed in output pipes: " + param);
		}
		for (vector<ParsedCall> &confNodes : call.confNodes) {
			replacementID = replaceForwardingCharactersAndConstants(file, confNodes, replacementID);
		}
		calls[callN] = call;
	}
	parse_validate(forwardedPipes.empty(), lastForwardIndex, "Hanging forwarding pipes left at end of function");
	return replacementID;
}

static bool isValidName(string &name) {
	if (name.empty()) return false;
	if (!isalpha(name[0])) return false; // Must start with letter
	for (char c : name) {
		if (c != '_' && !isdigit(c) && !isalpha(c)) return false; // numbers, letters, and underscores are allowed
	}
	return true;
}

static void validatePipeName(string &pipeName, int lineN) {
	if (pipeName == ">" || pipeName == "<" || pipeName == "?" || pipeName == "^") return;
	if (isConstant(pipeName)) {
		getConstantType(pipeName, lineN);
		return;
	}
	parse_validate(isValidName(pipeName), lineN, "Invalid pipe name: " + pipeName);
}
static void validateFunctionName(string &funcName, int lineN, bool inConfNodes) {
	if (inConfNodes) {
		if (isConstant(funcName)) {
			getConstantType(funcName, lineN);
			return;
		} /*else if (isDatatype(funcName)) {
			getDatatype(funcName, lineN);
			return;
		}*/ //TODO Enable datatype checks
	}
	parse_validate(isValidName(funcName), lineN, "Invalid function name: " + funcName);
}

static void validateCallList(vector<ParsedCall> &calls, bool inConfNodes) {
	for (ParsedCall &call : calls) {
		if (inConfNodes && isConstant(call.callName)) continue; // If a constant is encountered within a confNode list, skip

		for (string &pipe : call.inParams) {validatePipeName(pipe, call.lineN);}
		for (string &pipe : call.outParams) {validatePipeName(pipe, call.lineN);}
		for (vector<ParsedCall> &callList : call.confNodes) validateCallList(callList, true);
	}
}

inline void validatePipeAndFunctionNames(OthFile &file) {
	for (unsigned int i = 0; i < file.variables.size(); i++) { // Validate local/global variable names
		validatePipeName(file.variables[i], file.variable_lines[i]);
		validatePipeName(file.constants[i], file.constant_lines[i]);
	}
	for (Function &f : file.functionList) { // Validate names in function headers
		validateFunctionName(f.functionName, f.lineN, false);
		for (string pipe : f.variables) {
			validatePipeName(pipe, f.lineN);
		}
		for (string &confNode : f.confNodes) {
			validatePipeName(confNode, f.lineN);
		}
		validateCallList(f.callList, false);
	}
}

inline void replaceForwardingCharsAndConstants(OthFile &file) {
	for (Function &f : file.functionList) {
		for (string &var : f.variable_defaults) {
			if (var != "" && isConstant(var)) {
				var = file.getConstant(var, f.lineN); // Replace declaration constants
			}
		}
		replaceForwardingCharactersAndConstants(file, f.callList, 0);
	}
}

#endif
