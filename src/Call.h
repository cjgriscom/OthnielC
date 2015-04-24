#ifndef CALL_H_
#define CALL_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Datatypes.h>
#include <ConfigurationNode.h>
#include <VarReference.h>

class Call {
public:
	bool isBlockStart = false;
	bool isBlockEnd   = false;
	vector<Call> blockCalls;
	vector<VarReference> inputs;
	vector<VarReference> outputs;
	vector<ConfNode> confNodes;
	Function * callReference = NULL;

	uint32_t lineN;

};

#endif /* CALL_H_ */
