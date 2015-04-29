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
	VarReference reference; // Just CONSTANT

	ConfNode(bool isFuncReference, uint8_t declaredMode, vector<Call> newCallList) :
			mode(declaredMode),
			isReference(isFuncReference),
			calls(newCallList) {}

	bool isDeclaredReference() {return isReference;}
	OthFile * declaredReferenceFile() {return reference.othFile();}
	Function * declaredReferenceFunction() {return reference.function();}
	uint32_t declaredReferenceIndex() {return reference.index();}

	bool hasType() {return mode != CHAIN;}
	bool isDatatype() {return mode == DATATYPE;}
	bool isChain() {return mode == CHAIN;}
	bool isSingleOutputChain() {return mode == SOUT_CHAIN;}
	bool isConstant() {return mode == CONSTANT;}
	VarReference getReference() {return reference;}
	Datatype datatype() {return type;}
};

#endif /* CONFNODE_H_ */
