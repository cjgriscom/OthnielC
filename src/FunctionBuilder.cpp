#ifndef PIPELINKER_CPP_
#define PIPELINKER_CPP_

#include <OthUtil.h>
#include <Function.h>
#include <ParsedCall.h>
#include <string>
#include <vector>
using namespace std;

static Function parseDeclaration(Function f, ParsedCall call) {
	f.functionName = call.callName;
	f.nInputs = call.inParams.size();
	f.nOutputs = call.outParams.size();

	for (string in : call.inParams) {
		f.optionals.push_back(in.find("=") != in.npos); // If an input contains an = sign it's optional
	}

	for (uint32_t i = 0; i < f.nInputs+f.nOutputs; i++) {
		// Retrieve exp from either inParams or outParams
		string exp = i < f.nInputs ? call.inParams[i] : call.outParams[i-f.nInputs];

		//Syntax:  pipeName:I32 = 123
		int16_t colonPos = exp.find(':');
		parse_validate(colonPos != exp.npos, call.lineN, "Expected (label):(type)");

		string label = trim(exp.substr(0, colonPos));
		string datatypeS = trim(exp.substr(colonPos+1));

		int16_t equalPos = datatypeS.find('=');

		if (equalPos != datatypeS.npos) {
			string defaultExp = trim(datatypeS.substr(equalPos + 1)); // Separate default value
			datatypeS = trim(datatypeS.substr(0, equalPos)); // and datatype

			//Pipe constantPipe = Constants.matchConstant(defaultExp, lineN);
			//ParseError.validate(
			//		constantPipe.type().toString().equals(datatypeS), lineN,
			//		"Default value type does not match definition");

			//constantPipe.label = label;

			//return constantPipe;
		}
	}


	return f;
}

static Function beginFunction(uint32_t &i, vector<ParsedCall> calls) {
	uint8_t rm = INVALID;
	uint8_t mm = INVALID;
	Function f;

	for (; i < calls.size(); i++) { // Use index from assembleFunctions()
		ParsedCall call = calls[i];
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
			f = parseDeclaration(f, call);

			// Advance and return
			i++; break;
		}
	}
	return f;

}

static vector<Function> assembleFunctions(vector<ParsedCall> calls) {
	vector<Function> functions;

	bool inFunction = false;

	Function f;

	for (uint32_t i = 0; i < calls.size(); i++) {
		ParsedCall call = calls[i];
		if (!inFunction) { // Needs to start with a declaration
			parse_validate(qualifiesAsKeyword(call), call.lineN, "Expected function declaration");
		}
		if (qualifiesAsKeyword(call) && is_function_kw(call.callName)) { // Check for a header keyword
			f = beginFunction(i, calls);
			inFunction = true;
		}


	}

	return functions;
}


#endif
