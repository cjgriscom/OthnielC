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
#include <sstream> // for to_string

using namespace std;

static int replaceForwardingCharacters(vector<ParsedCall> &calls, int startID) { //TODO I think this might have an issue with blocks....
	queue<string> forwardedPipes;
	int replacementID = startID;
	ParsedCall call;
	int lastForwardIndex = 0;
	for (int callN = 0; callN < calls.size(); callN++) {
		call = calls[callN];
		for (string &param : call.inParams) {
			parse_validate(param != ">", call.lineN, "Encountered > pipe in inputs");
			if (param == "<") {
				parse_validate(!forwardedPipes.empty(), call.lineN, "Encountered < but no forwarded pipes exist!");
				param = forwardedPipes.front();
				forwardedPipes.pop();
			}
		}
		for (string &param : call.outParams) {
			parse_validate(param != "<", call.lineN, "Encountered < pipe in outputs");
			if (param == ">") {
				std::stringstream ss;
				ss << "_fwd_" << replacementID++ << "_" << call.lineN;
				param = ss.str();
				forwardedPipes.push(ss.str());
				lastForwardIndex = call.lineN; //for error messages if needed
			}
		}
		for (vector<ParsedCall> &confNodes : call.confNodes) {
			replacementID = replaceForwardingCharacters(confNodes, replacementID);
		}
		calls[callN] = call;
	}
	parse_validate(forwardedPipes.empty(), lastForwardIndex, "Hanging forwarding pipes left at end of function");
	return replacementID;
}

static void setPipeCodes(OthFile &file) {
	//TODO first validate pipe names (so that we can start inserting illegal names like < replacements)
	//		i.e. _ as first character is illegal
	for (Function &f : file.functionList) {
		replaceForwardingCharacters(f.callList, 0);
	}
}

#endif
