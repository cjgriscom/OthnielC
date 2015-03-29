#ifndef PARSEDCALL_H_
#define PARSEDCALL_H_

#include <string>
#include <vector>
#include <OthUtil.h>
using namespace std;

struct ParsedCall {
	uint32_t lineN;
	bool isBlockStart = false;
	bool isBlockEnd = false;
	vector<string> inParams;
	vector<string> outParams;
	vector<string> auxVars;
	vector<vector<ParsedCall>> confNodes;
	string callName = "UNDEFINED";
};

#define qualifiesAsKeyword(x) (x.inParams.size() == 0 && x.outParams.size() == 0 && x.auxVars.size() == 0 && x.confNodes.size() == 0 && !x.isBlockStart && !x.isBlockEnd)

#endif /* PARSEDCALL_H_ */
