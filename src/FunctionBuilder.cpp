#ifndef PIPELINKER_CPP_
#define PIPELINKER_CPP_

#include <OthUtil.h>
#include <Function.h>
#include <ParsedCall.h>
#include <string>
#include <vector>
using namespace std;

static Function f;

static void parseDeclaration(ParsedCall *callRef) { //TODO add confnodes
	ParsedCall call = *callRef;

	f.functionName = call.callName;
	f.nInputs = call.inParams.size();
	f.nOutputs = call.outParams.size();

	for (uint32_t i = 0; i < f.nInputs+f.nOutputs+call.auxVars.size(); i++) {
		// Retrieve exp from either inParams or outParams or auxVars
		string exp = i < f.nInputs ? call.inParams[i] :
				(i < f.nInputs + f.nOutputs ? call.outParams[i-f.nInputs] :
				call.auxVars[i-f.nInputs-f.nOutputs]);

		//Syntax:  pipeName:I32 = 123
		int16_t colonPos = exp.find(':');
		parse_validate(colonPos != exp.npos, call.lineN, "Expected (label):(type)");

		string label = trim(exp.substr(0, colonPos));
		string datatypeS = trim(exp.substr(colonPos+1));
		string defaultValue = ""; // No default

		int16_t equalPos = datatypeS.find('=');

		if (equalPos != datatypeS.npos) { // If we found an equals expression
			defaultValue = trim(datatypeS.substr(equalPos + 1)); // Separate default value
			datatypeS = trim(datatypeS.substr(0, equalPos)); // and datatype
		}

		f.variables.push_back(label);
		f.variable_types.push_back(datatypeS);
		f.variable_defaults.push_back(defaultValue);
	}

}

static int beginFunction(unsigned int i, vector<ParsedCall> *calls) {
	uint8_t rm = INVALID;
	uint8_t mm = INVALID;

	for (; i < (*calls).size(); i++) { // Use index from assembleFunctions()
		ParsedCall call = (*calls)[i];

		if (uint8_t test = rm_ID(call.callName) != INVALID) { // Check for correct behavior... TODO
			parse_validate(rm == INVALID, call.lineN, "Too many run mode keywords in declaration");
			rm = test;
		} else if (uint8_t test = mm_ID(call.callName) != INVALID) {
			parse_validate(mm == INVALID, call.lineN, "Too many memory mode keywords in declaration");
			mm = test;
		} else {
			parse_validate(rm != INVALID && mm != INVALID, call.lineN, "Incomplete declaration");
			f.runMode = rm;
			f.memoryMode = mm;

			// calls(i) should now contain function declaration and stuff.
			parseDeclaration(&call);

			// Advance and return
			//i++; XXX
			return i;
		}
	}
	return i;

}

static AbstractCall ACFromPC(ParsedCall call) {
	AbstractCall ac;
	ac.blockStart = call.isBlockStart; ac.blockEnd = call.isBlockEnd;
	ac.callID = call.callName;
	ac.inPipes = call.inParams; ac.outPipes = call.outParams;
	for (vector<ParsedCall> pcList : call.confNodes) {
		vector<AbstractCall> acList;
		for (ParsedCall pc : pcList) {
			acList.push_back(ACFromPC(pc));
		}
		ac.confNodes.push_back(acList);
	}
	return ac;
}

static void assembleFunctions(vector<Function> * fnctns, vector<ParsedCall> *calls) {
	bool inFunction = false;

	for (uint32_t i = 0; i < (*calls).size(); i++) {
		ParsedCall call = (*calls)[i];
		if (!inFunction) { // Needs to start with a declaration
			parse_validate(qualifiesAsKeyword(call), call.lineN, "Expected function declaration");
		}
		if (qualifiesAsKeyword(call) && is_function_kw(call.callName)) { // Check for a header keyword
			if (inFunction) (*fnctns).push_back(f); // Push previous function
			f = Function();
			i = beginFunction(i, calls);
			inFunction = true;
		} else {
			f.callList.push_back(ACFromPC(call));
		}
	}
	(*fnctns).push_back(f);
}

static void printCallsFB(int indent, vector<AbstractCall> *calls, char delim) {
	for (AbstractCall call : *calls) {
		if (call.blockEnd) {
			indent--;
		}
		for (int ind = 0; ind < indent; ind++) {
			cout << "  ";
		}
		if (call.blockEnd)
			cout << ":";
		cout << "[";
		for (string param : call.inPipes) {
			cout << param << " ";
		}
		cout << "]";
		cout << call.callID;
		if (call.confNodes.size() > 0) {
			cout << "{ ";
			for (vector<AbstractCall> param : call.confNodes) {
				printCallsFB(0, &param, ' ');
			}
			cout << "}";
		}

		cout << "[";
		for (string param : call.outPipes) {
			cout << param << " ";
		}
		cout << "]";
		if (call.blockStart) {
			indent++;
			cout << ":";
		}
		cout << delim;
	}
}

static void testFB(vector<Function> * fnctns) {
	for (Function fn : *fnctns) {
		bool hasAux = false;
		cout << mm_kw(fn.memoryMode) << " " << rm_kw(fn.runMode) << " ";
		for (int i = 0; i < fn.variables.size(); i++) {
			if (fn.nInputs == 0) cout << fn.functionName;
			if ((i == 0 && fn.nInputs > 0) || (i == fn.nInputs && fn.nOutputs > 0)) cout << "[";
			if (i == fn.nInputs+fn.nOutputs) {cout << " <"; hasAux=true;}
			cout << fn.variables[i] << ":" << fn.variable_types[i];
			if (fn.variable_defaults[i].size() > 0) cout << "=" << fn.variable_defaults[i];
			bool printAuxBracket = i == fn.variables.size()-1 && hasAux;
			if (i == fn.nInputs-1 || i == fn.nInputs+fn.nOutputs-1) cout << "]";
			else if (!printAuxBracket) cout << ", ";
			if (i == fn.nInputs-1) cout << fn.functionName;
			if (printAuxBracket) cout << ">";
		}
		hasAux = false;
		cout << endl;
		printCallsFB(1, &(fn.callList), '\n');
	}
}


#endif
