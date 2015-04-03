#ifndef DATATYPES_H_
#define DATATYPES_H_

#include<OthUtil.h>

//Datatypes
const uint8_t U8 = 0x00;
const uint8_t I8 = 0x01;
const uint8_t U16 = 0x02;
const uint8_t I16 = 0x03;
const uint8_t U32 = 0x04;
const uint8_t I32 = 0x05;
const uint8_t U64 = 0x06;
const uint8_t I64 = 0x07;

const uint8_t F32 = 0x08; // Float
const uint8_t F64 = 0x09; // Double
const uint8_t F80 = 0x0A; // Long double

const uint8_t BOOL = 0x10;
const uint8_t STRING = 0x11;
const uint8_t CHAR = 0x11;

const uint8_t ARRAY = 0x70;
const uint8_t CLUSTER = 0x71;

// Abstract types
const uint8_t INTEGER  = 0xFD;
const uint8_t NUMERIC  = 0xFE;
const uint8_t ANYTHING = 0xFF;

class AbstractDatatype {
public:
	uint8_t typeConstant = 0;

	AbstractDatatype(uint8_t typeConstant) {
		this->typeConstant = typeConstant;
	}
	bool isAbstract() {return true;}
	bool isSimpleType() {return true;}
	string asString() {
		return "Abstract";
	}
};

class Datatype : public AbstractDatatype {
public:
	Datatype(uint8_t typeConstant) : AbstractDatatype(typeConstant) {}
	bool isAbstract() {return false;}
};

class ArrayType : public Datatype {
	Datatype baseType;
	int dimensions;
	ArrayType(Datatype &baseType, int dimensions) :
			Datatype(ARRAY),
			baseType(baseType),
			dimensions(dimensions) {}
	bool isAbstract() {return baseType.isAbstract();}
	bool isSimpleType() {return false;}
};

class ClusterType : public Datatype {
	Datatype * types;
	int nTypes = 0;
	ClusterType(int nTypes, Datatype baseTypes[]) :
			Datatype(CLUSTER),
			types(baseTypes),
			nTypes(nTypes) {}
	bool isAbstract() {
		for (int i = 0; i < nTypes; i++) {
			if (types[i].isAbstract()) return true;
		}
		return false;
	}
	bool isSimpleType() {return false;}
};

static inline Datatype evaluateDatatype(string expression, uint64_t lineN) {

}

#endif /* DATATYPES_H_ */
