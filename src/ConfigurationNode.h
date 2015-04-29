#ifndef CONFNODE_H_
#define CONFNODE_H_

#include <vector>
#include <stack>
#include <string>
#include <OthUtil.h>
#include <Datatypes.h>
#include <Keywords.h>
#include <VarReference.h>

class ConfNode {
	uint8_t mode;
	bool isReference;

public:
	Datatype type = Datatype(BOOL); // Common to all but CHAIN
	vector<Call> calls; // Common to CHAIN and SOUT_CHAIN
	VarReference constantReference; // Just CONSTANT

	ConfNode(bool isFuncReference, uint8_t declaredMode, vector<Call> newCallList) :
			mode(declaredMode),
			isReference(isFuncReference),
			calls(newCallList) {}

	bool hasType() {return mode != CHAIN;}
	bool isDeclaredReference() {return isReference;}
	bool isDatatype() {return mode == DATATYPE;}
	bool isChain() {return mode == CHAIN;}
	bool isSingleOutputChain() {return mode == SOUT_CHAIN;}
	void update_SOUT_CHAIN(Datatype outtype) {type = outtype;}
	bool isConstant() {return mode == CONSTANT;}
	//OthFile * othFile() {return file;}
	//Function * function() {return func;}
	//Call * callRef() {return call;}
	Datatype datatype() {return type;}
};

#endif /* CONFNODE_H_ */
