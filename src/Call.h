#ifndef CALL_H_
#define CALL_H_

class Call;

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Function.h>
#include <VarReference.h>
#include <ConfigurationNode.h>
#include <Datatypes.h>

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
