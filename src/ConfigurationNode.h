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

	// Common to all types
	//OthFile * file;
	//Function * func = NULL;
	//Call * call = NULL;
	bool isReference = false;

public:
	Datatype type = Datatype(BOOL); // Common to all but CHAIN
	vector<Call> calls; // Common to CHAIN and SOUT_CHAIN
	VarReference constantReference; // Just CONSTANT

	ConfNode(OthFile &file, Function &function, uint32_t lineN, vector<ParsedCall> &parsedNode, uint8_t declaredMode, vector<Call> newCallList, stack<vector<Call>*> &blockStack) :
			mode(declaredMode),
			calls(newCallList) {
		// TODO first, if if qualifies as a keyword see if it references something in the function declaration

		if (declaredMode == CHAIN || declaredMode == SOUT_CHAIN) {
			// Handled in reference resolver
		} else {
			parse_validate(parsedNode.size() == 1 && qualifiesAsKeyword_strict(parsedNode[0]), lineN, "Configuration node is not a valid " + string(cn_kw(declaredMode)) + " expression");
			ParsedCall data = parsedNode[0];

			if (declaredMode == DATATYPE) {

			} else if (declaredMode == CONSTANT) {

			}
		}
	}

	/*ConfNode(VarReference constant) :
			mode(CONSTANT),
			file(constant.othFile()),
			func(constant.function()),
			type(constant.datatype()),
			constantReference(constant)
			{}*/

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
