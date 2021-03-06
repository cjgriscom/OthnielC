#ifndef VARREFERENCE_H_
#define VARREFERENCE_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <Datatypes.h>
#include <OthFile.h>
#include <Function.h>

static pair<uint8_t,uint32_t> vr_Construct_func(Function * func, uint32_t index, Datatype * type);

#define VAR_JUNK 0
#define VAR_PIPE 1
#define VAR_IN   2
#define VAR_OUT  3
#define VAR_AUX  4
#define VAR_GLOB 5
#define VAR_CNST 6
#define VAR_BLCK 7 //TODO
#define VAR_SELF 8
#define VAR_NEW  9

class VarReference {
	uint8_t mode;

	bool garbageOrOptional = false;

	OthFile * file;         // Common to all 3
	Function * func = NULL; // Common to VAR_FUNC and VAR_PIPE
	Call * call = NULL;     // Only VAR_PIPE

	uint32_t i = 0; // Common to all 3
	vector<uint32_t> subIndices; // Only VAR_BLCK
	Datatype type = Datatype(ANYTHING);

public:

	string name;

	VarReference() : mode(VAR_JUNK), type(Datatype(ANYTHING)) { // ^ or ? pipe
		file = NULL;
		garbageOrOptional = true;
	}

	// Pipe reference or self reference
	VarReference(string name, OthFile * file, Function * function, Call * callRef, uint32_t index, Datatype type, bool selfRef) :
			mode(selfRef ? VAR_SELF : VAR_PIPE),
			file(file),
			func(function),
			call(callRef),
			i(index),
			type(type),
			name(name) {}

	// Constant OR Variable
	VarReference(string name, bool isConstant, OthFile * file, uint32_t index) :
			mode(isConstant ? VAR_CNST : VAR_GLOB),
			file(file),
			i(index),
			type((isConstant ? file->constant_types : file->variable_types)[index]),
			name(name) {}

	// In/out/aux
	VarReference(string name, OthFile * file, Function * function, uint32_t index) :
			file(file),
			func(function),
			name(name) {
		pair<uint8_t,uint32_t> p = vr_Construct_func(function, index, &type);
		mode = p.first;
		i = p.second;
	}

	// New
	VarReference(string name, OthFile * file, Function * function, Datatype type) :
			mode(VAR_NEW),
			file(file),
			func(function),
			type(type),
			name(name) {}

	// ConfNode cheat constructor (internal)
	VarReference(OthFile * file, Function * function, uint32_t index) :
			file(file),
			func(function) {
		mode = -1;
		i = index;
	}

	bool isOptional() {return garbageOrOptional;}
	bool isGarbage() {return garbageOrOptional;}
	bool isPipe() {return mode == VAR_PIPE;}
	bool isF_In() {return mode == VAR_IN;}
	bool isF_Out() {return mode == VAR_OUT;}
	bool isF_Aux() {return mode == VAR_AUX;}
	bool isGlobal() {return mode == VAR_GLOB;}
	bool isConstant() {return mode == VAR_CNST;}
	bool isSelfRef() {return mode == VAR_SELF;}
	OthFile * othFile() {return file;}
	Function * function() {return func;}
	Call * callRef() {return call;}
	uint32_t index() {return i;}
	Datatype datatype() {return type;}
};

#endif /* VARREFERENCE_H_ */
