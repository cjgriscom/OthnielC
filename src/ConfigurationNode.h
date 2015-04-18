#ifndef CONFNODE_H_
#define CONFNODE_H_

class ConfNode;
class Call;

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Function.h>
#include <VarReference.h>
#include <Datatypes.h>
#include <Keywords.h>

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

	//ConfNode(VarReference constant) : constantReference(constant) {}

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
