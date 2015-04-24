#ifndef VARREFERENCE_H_
#define VARREFERENCE_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Datatypes.h>
#include <OthFile.h>

#define VAR_JUNK 0
#define VAR_PIPE 1
#define VAR_FUNC 2
#define VAR_GLOB 3
#define VAR_CNST 4
#define VAR_BLCK 5

class VarReference {
	uint8_t mode;

	bool garbageOrOptional = false;

	OthFile * file;         // Common to all 3
	Function * func = NULL; // Common to VAR_FUNC and VAR_PIPE
	Call * call = NULL;     // Only VAR_PIPE

	uint32_t i = 0; // Common to all 3
	vector<uint32_t> subIndices; // Only VAR_BLCK
	Datatype type;

public:
	VarReference() : mode(VAR_JUNK), type(Datatype(ANYTHING)) { // ^ or ? pipe
		file = NULL;
		garbageOrOptional = true;
	}

	// Pipe
	VarReference(OthFile * file, Function * function, Call * callRef, uint32_t outIndex, Datatype &type) :
			mode(VAR_PIPE),
			file(file),
			type(type),
			func(function),
			call(callRef),
			i(outIndex) {}

	// Constant OR Variable
	VarReference(bool isConstant, OthFile * file, uint32_t index) :
			mode(isConstant ? VAR_CNST : VAR_GLOB),
			file(file),
			type((isConstant ? file->constant_types : file->variable_types)[index]),
			i(index) {}

	bool isOptional() {return garbageOrOptional;}
	bool isGarbage() {return garbageOrOptional;}
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
