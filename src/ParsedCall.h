#ifndef PARSEDCALL_H_
#define PARSEDCALL_H_

#include <string>
#include <vector>
#include <OthUtil.h>
using namespace std;

class Function; // Forward declaration of Function for resolver

struct ParsedCall {
	uint32_t lineN;
	bool isBlockStart = false;
	bool isBlockEnd = false;
	vector<string> inParams;
	vector<string> outParams;
	vector<string> auxVars; // For internal use
	vector<vector<ParsedCall>> confNodes;
	string callName = "UNDEFINED";

	// For resolution stage
	Function * callReference = NULL;
	bool callResolved = false;
};

#define qualifiesAsKeyword(x) (x.inParams.size() == 0 && x.outParams.size() == 0 && !x.isBlockStart && !x.isBlockEnd)
#define qualifiesAsKeyword_strict(x) (qualifiesAsKeyword(x) && x.confNodes.size() == 0 && x.auxVars.size() == 0)
#endif /* PARSEDCALL_H_ */
