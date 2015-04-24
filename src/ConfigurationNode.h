#ifndef CONFNODE_H_
#define CONFNODE_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Datatypes.h>
#include <Keywords.h>
#include <VarReference.h>

class ConfNode {
	uint8_t mode;

	// Common to all types
	OthFile * file;
	Function * func = NULL;
	Call * call = NULL;

public:
	Datatype type; // Common to all but CHAIN
	vector<Call> calls; // Common to CHAIN and SOUT_CHAIN
	VarReference constantReference; // Just CONSTANT

	ConfNode(VarReference constant) :
			mode(CONSTANT),
			file(constant.othFile()),
			func(constant.function()),
			type(constant.datatype()),
			constantReference(constant)
			{}

	bool hasType() {return mode != CHAIN;}
	bool isDatatype() {return mode == DATATYPE;}
	bool isChain() {return mode == CHAIN;}
	bool isSingleOutputChain() {return mode == SOUT_CHAIN;}
	bool isConstant() {return mode == CONSTANT;}
	OthFile * othFile() {return file;}
	Function * function() {return func;}
	Call * callRef() {return call;}
	Datatype datatype() {return type;}
	Datatype satisfiedDatatype(vector<Datatype> callInputs) { // Add the option of passing the call stack??
		//TODO
	}
};

#endif /* CONFNODE_H_ */
