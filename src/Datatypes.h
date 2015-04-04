#ifndef DATATYPES_H_
#define DATATYPES_H_

#include<OthUtil.h>
#include<algorithm>

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
const uint8_t CHAR = 0x12;

const uint8_t ARRAY = 0x70;
const uint8_t CLUSTER = 0x71;

// Abstract types
const uint8_t TYPEOF   = 0xFB;
const uint8_t STRONGESTOF = 0xFC;
const uint8_t INTEGER  = 0xFD;
const uint8_t NUMERIC  = 0xFE;
const uint8_t ANYTHING = 0xFF;

class Datatype {
	Datatype * baseType = NULL;  // Array: Base type
	int dimensions = 0;          // Array: Dimensions

	vector<Datatype> types;      // Cluster: Array of types

	vector<uint32_t> varRefs;    // strongestof or typeof references

public:
	uint8_t typeConstant = 0;

	Datatype(uint8_t typeConstant) : typeConstant(typeConstant) {}
	Datatype(Datatype * baseType, int dimensions) : typeConstant(ARRAY) {
		this->baseType = baseType;
		this->dimensions = dimensions;
	}
	Datatype(int nTypes, vector<Datatype> baseTypes) : typeConstant(CLUSTER) {
		this->types = baseTypes;
	}
	Datatype(uint32_t variableIndex, bool dummy) : typeConstant(TYPEOF) {
		varRefs.push_back(variableIndex);
	}
	Datatype(vector<uint32_t> variables, int nVariables) : typeConstant(STRONGESTOF) {
		varRefs = variables;
	}

	bool isAbstract() {
		if (typeConstant == ARRAY) {
			return (*baseType).isAbstract();
		} else if (typeConstant == CLUSTER) {
			for (unsigned int i = 0; i < types.size(); i++) {
				if (types[i].isAbstract()) return true;
			}
			return false;
		} else {
			return typeConstant >= TYPEOF;
		}
	}
	string asString() {
		if (typeConstant == ARRAY) {
			return baseType->asString() + "(" + intToString(dimensions) + ")";
		} else if (typeConstant == CLUSTER){
			return "CLUSTER"; //TODO
		} else if (typeConstant == TYPEOF) {
			return "typeof(" + intToString(varRefs[0]) + ")";
		} else if (typeConstant == STRONGESTOF) {
			string expr = "strongestof(";
			for (unsigned int i = 0; i < varRefs.size(); i++) {
				if (i != 0) expr += ", ";
				expr += intToString(varRefs[i]);
			}
			return expr + ")";
		} else if (isAbstract()) {
			return (typeConstant==INTEGER ? "integer" : (typeConstant == NUMERIC ? "numeric" : (typeConstant == ANYTHING ? "anything" : "???")));
		} else {
			switch (typeConstant) {
				case U8: return "U8";
				case I8: return "I8";
				case U16: return "U16";
				case I16: return "I16";
				case U32: return "U32";
				case I32: return "I32";
				case U64: return "U64";
				case I64: return "I64";
				case F32: return "F32";
				case F64: return "F64";
				case F80: return "F80";
				case BOOL: return "boolean";
				case STRING: return "string";
				case CHAR: return "char";
				default: return "???";
			}
		}
	}
};

inline bool verifyLen(string &expression, unsigned int len, uint32_t lineN) {
	parse_validate(expression.size() >= len, lineN, "Could not parse datatype expression: " + expression);
	return true;
}

static inline Datatype evaluateDatatype(string expression, uint32_t lineN, vector<string> inputNames) {
	verifyLen(expression, 2, lineN); // Make sure it's not empty

	if (expression.find("typeof(") == 0 && expression[expression.size()-1] == ')') {
		expression = trim(expression.substr(7, expression.size()-8));
		for (unsigned int i = 0; i < inputNames.size(); i++) {
			if (inputNames[i] == expression) return Datatype(i, true);
		}
		parse_validate(false, lineN, "Parameter " + expression + " could not be found in inputs");
	} else if (expression.find("strongestof(") == 0 && expression[expression.size()-1] == ')') {
		expression = expression.substr(12, expression.size()-13);
		size_t n = std::count(expression.begin(), expression.end(), ',');
		//TODO
	}

	bool isInt = false;
	unsigned int type = 0;
	unsigned int requiredLength = 3;
	if (expression[0] == 'U') {
		isInt = true; requiredLength = 2;
	} else if (expression[0] == 'I') {
		isInt = true; requiredLength = 2; type++;
	}
	verifyLen(expression, requiredLength, lineN); // Check size
	if (isInt && expression.find("8") == 1) {type += 0; requiredLength = 2;}
	else if (isInt && expression.find("16") == 1) {type += 2; requiredLength = 3;}
	else if (isInt && expression.find("32") == 1) {type += 4; requiredLength = 3;}
	else if (isInt && expression.find("64") == 1) {type += 6; requiredLength = 3;}
	else if (expression.find("F32") == 0) {type = F32; requiredLength = 3;}
	else if (expression.find("F64") == 0) {type = F64; requiredLength = 3;}
	else if (expression.find("F80") == 0) {type = F80; requiredLength = 3;}
	else if (expression.find("boolean") == 0) {type = BOOL; requiredLength = 7;}
	else if (expression.find("string") == 0) {type = STRING; requiredLength = 6;}
	else if (expression.find("char") == 0) {type = CHAR; requiredLength = 4;}
	else if (expression.find("integer") == 0) {type = INTEGER; requiredLength = 7;}
	else if (expression.find("numeric") == 0) {type = NUMERIC; requiredLength = 7;}
	else if (expression.find("anything") == 0) {type = ANYTHING; requiredLength = 8;}

	if (expression.length() == requiredLength) return Datatype(type); // Not an array

	 //TODO Arrays and clusters
	parse_validate(false, lineN, "Invalid or unimplemented datatype: " + expression);
	return Datatype(BOOL);
}

static inline Datatype evaluateDatatypeWithoutAbstracts(string expression, uint32_t lineN, string errorMessage) {
	vector<string> emptyVector;
	Datatype dt = evaluateDatatype(expression, lineN, emptyVector);
	parse_validate(!dt.isAbstract(), lineN, errorMessage);
	return dt;
}

#endif /* DATATYPES_H_ */
