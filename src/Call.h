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

	vector<Datatype> input_types() {
		vector<Datatype> typeList;
		for (VarReference vr : inputs) typeList.push_back(vr.datatype());
		return typeList;
	}

	vector<Datatype> confNode_types() {
		vector<Datatype> typeList;
		for (ConfNode cn : confNodes) typeList.push_back(cn.type);
		return typeList;
	}

	vector<Datatype> output_types() {
		vector<Datatype> typeList;
		for (VarReference vr : inputs) typeList.push_back(vr.datatype());
		return typeList;
	}

};

#endif /* CALL_H_ */
