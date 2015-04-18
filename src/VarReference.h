/*
 * VarReference.h
 *
 *  Created on: Apr 10, 2015
 *      Author: chandler
 */

#ifndef VARREFERENCE_H_
#define VARREFERENCE_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Function.h>
#include <Datatypes.h>

class VarReference;
class Call;

#define VAR_PIPE 0
#define VAR_FUNC 1
#define VAR_GLOB 2
#define VAR_CNST 3
#define VAR_BLCK 4

class VarReference {
	uint8_t mode;

	OthFile * file;             // Common to all 3
	Function * func = NULL; // Common to VAR_FUNC and VAR_PIPE
	Call * call = NULL;   // Only VAR_PIPE

	uint32_t i = 0; // Common to all 3
	vector<uint32_t> subIndices; // Only VAR_BLCK
	Datatype type;

public:
	bool isPipe() {return mode == VAR_PIPE;}
	bool isFunction() {return mode == VAR_FUNC;}
	bool isGlobal() {return mode == VAR_GLOB;}
	bool isConstant() {return mode == VAR_CNST;}
	OthFile * othFile() {return file;}
	Function * function() {return func;}
	Call * callRef() {return call;}
	uint32_t index() {return i;}
	Datatype datatype() {return type;}
	Datatype satisfiedDatatype(vector<Datatype> callInputs) { // Add the option of passing the call stack
		//TODO
	}
};

#endif /* VARREFERENCE_H_ */
