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
const uint8_t NODE     = 0xFA;
const uint8_t TYPEOF   = 0xFB;
const uint8_t STRONGESTOF = 0xFC;
const uint8_t INTEGER  = 0xFD;
const uint8_t NUMERIC  = 0xFE;
const uint8_t ANYTHING = 0xFF;

class Datatype {
	int dimensions = 0;          // Array: Dimensions

	vector<Datatype> types;      // Array: Base type, or Cluster: Array of types
public:
	vector<uint32_t> varRefs;    // strongestof, typeof, or node references
	uint8_t typeConstant = 0;

	Datatype(uint8_t typeConstant) : typeConstant(typeConstant) {}
	Datatype(Datatype baseType, int dimensions) : typeConstant(ARRAY) {
		types.push_back(baseType);
		this->dimensions = dimensions;
	}
	Datatype(int nTypes, vector<Datatype> baseTypes) : typeConstant(CLUSTER) {
		this->types = baseTypes;
	}
	Datatype(uint32_t variableIndex, bool node) : typeConstant(node ? NODE : TYPEOF) {
		varRefs.push_back(variableIndex);
	}
	Datatype(vector<uint32_t> variables) : typeConstant(STRONGESTOF) {
		varRefs = variables;
	}

	bool isSigned() {
		return typeConstant <= I64 && typeConstant % 2 == 1;
	}

	bool isNumeric(bool allowAbstract) {
		if (allowAbstract && (typeConstant == INTEGER || typeConstant == NUMERIC)) return true;
		return typeConstant <= F80;
	}

	bool isInteger(bool allowAbstract) {
		if (allowAbstract && typeConstant == INTEGER) return true;
		return typeConstant <= I64;
	}

	bool isAbstract() {
		if (typeConstant == ARRAY) {
			return types[0].isAbstract();
		} else if (typeConstant == CLUSTER) {
			for (unsigned int i = 0; i < types.size(); i++) {
				if (types[i].isAbstract()) return true;
			}
			return false;
		} else {
			return typeConstant >= NODE;
		}
	}

	bool isIndependantAbstract() {
		if (typeConstant == ARRAY) {
			return types[0].isIndependantAbstract();
		} else if (typeConstant == CLUSTER) {
			for (unsigned int i = 0; i < types.size(); i++) {
				if (types[i].isIndependantAbstract()) return true;
			}
			return false;
		} else {
			return typeConstant >= INTEGER;
		}
	}

	#define DT_INCOMPATIBLE 0
	#define DT_CASTABLE     1  // Order: functionType.getCompatibilityValue(callType)
	#define DT_COMPATIBLE   2  // Order: functionType.getCompatibilityValue(callType)
	#define DT_EQUAL        3
	uint32_t getCompatibilityValue(Datatype other) {
		if (typeConstant == ANYTHING) return DT_COMPATIBLE;

		if (typeConstant == ARRAY && other.typeConstant == ARRAY) {
			return dimensions == other.dimensions ? types[0].getCompatibilityValue(other.types[0]) : DT_INCOMPATIBLE;
		}
		if (typeConstant == CLUSTER && other.typeConstant == CLUSTER) {
			if (types.size() != other.types.size()) return DT_INCOMPATIBLE;
			uint32_t minValue = DT_EQUAL;
			for (uint32_t i = 0; i < types.size(); i++) {
				uint32_t compat = types[i].getCompatibilityValue(other.types[i]);
				if (compat < minValue) minValue = compat;
			}
			return minValue;
		}

		if (typeConstant == other.typeConstant) return DT_EQUAL;

		if ((typeConstant == NUMERIC || typeConstant == F80) && other.isNumeric(true)) return DT_COMPATIBLE;
		if (typeConstant == INTEGER && other.isInteger(true)) return DT_COMPATIBLE;
		if (typeConstant == INTEGER && other.isNumeric(true)) return DT_CASTABLE;

		if (other.typeConstant == NUMERIC && isNumeric(false)) return DT_CASTABLE;
		if (other.typeConstant == INTEGER && isNumeric(false)) return DT_CASTABLE;

		if (isNumeric(false) && other.isNumeric(false)) {
			if (isInteger(false)) {
				if (other.isInteger(false)) {
					if (isSigned()) {
						return typeConstant > other.typeConstant + 1 ? DT_COMPATIBLE : DT_CASTABLE;
					} else {
						if (other.isSigned()) {
							return DT_CASTABLE;
						} else {
							return typeConstant > other.typeConstant ? DT_COMPATIBLE : DT_CASTABLE;
						}
					}
				} else {
					return DT_CASTABLE;
				}
			} else {
				return typeConstant > other.typeConstant ? DT_COMPATIBLE : DT_CASTABLE;
			}
		}

		return DT_INCOMPATIBLE;
	}

	static Datatype getStrongestofCombination(vector<Datatype> inputTypes, uint32_t lineN) {
		if (inputTypes[0].typeConstant == ARRAY) {
			vector<Datatype> innerTypes;
			for (Datatype arr : inputTypes) innerTypes.push_back(arr.types[0]);
			return Datatype(getStrongestofCombination(innerTypes, lineN), inputTypes[0].dimensions);
		} else if (inputTypes[0].typeConstant == CLUSTER) {
			//TODO
			return inputTypes[0];
		} else {
			Datatype strongest = inputTypes[0];
			for (uint32_t i = 1; i < inputTypes.size(); i++) {
				parse_validate(strongest.getCompatibilityValue(inputTypes[i]) >= DT_CASTABLE &&
						inputTypes[i].getCompatibilityValue(strongest) >= DT_CASTABLE,
						lineN,
						"Incompatible types specified for strongestof() expression");
				if (inputTypes[i].typeConstant > strongest.typeConstant) strongest = inputTypes[i];
			}
			return strongest;
		}
	}

	Datatype nextSatisfiedType(vector<Datatype> func_input_types, vector<Datatype> call_input_types, vector<Datatype> call_confNode_types, uint32_t declLine, uint32_t lineN) {
		Datatype self = *this;
		Datatype newType = self;

		if (isAbstract()) {
			if (typeConstant == ARRAY) {
				return Datatype(self.types[0].nextSatisfiedType(func_input_types, call_input_types, call_confNode_types, declLine, lineN), self.dimensions);
			} else if (typeConstant == CLUSTER) {
				vector<Datatype> baseTypes;
				for (Datatype &ref : self.types) {
					baseTypes.push_back(ref.nextSatisfiedType(func_input_types, call_input_types, call_confNode_types, declLine, lineN));
				}
				return Datatype(baseTypes.size(), baseTypes);
			} else if (typeConstant == TYPEOF) {
				newType = call_input_types[self.varRefs[0]];
				parse_validate(!func_input_types[self.varRefs[0]].isAbstract() || func_input_types[self.varRefs[0]].isIndependantAbstract(), declLine, "A typeof() expression must reference a concrete or independent abstract type");
			} else if (typeConstant == STRONGESTOF) {
				vector<Datatype> callTypes;
				vector<Datatype> funcTypes;
				for (uint32_t reference : self.varRefs) {
					callTypes.push_back(call_input_types[reference]);
					funcTypes.push_back(func_input_types[reference]);
				}
				for (Datatype check : funcTypes) parse_validate(!check.isAbstract() || check.isIndependantAbstract(), declLine, "A strongestof() expression must reference concrete or independent abstract types");
				newType = getStrongestofCombination(callTypes, lineN);
			} else if (typeConstant == NODE) {
				newType = call_confNode_types[self.varRefs[0]];
				parse_validate(!newType.isAbstract(), lineN, "ConfNode reference does not name a valid datatype");
			}
		}
		return newType;
	}

	string asString() {
		if (typeConstant == ARRAY) {
			return types[0].asString() + "(" + intToString(dimensions) + ")";
		} else if (typeConstant == CLUSTER){
			string expr = "(";
			for (uint32_t i = 0; i < types.size(); i++) {
				if (i != 0) expr += ", ";
				expr += types[i].asString();
			}
			return expr + ")";
		} else if (typeConstant == TYPEOF) {
			return "typeof(" + intToString(varRefs[0]) + ")";
		} else if (typeConstant == NODE) {
			return "node(" + intToString(varRefs[0]) + ")";
		} else if (typeConstant == STRONGESTOF) {
			string expr = "strongestof(";
			for (uint32_t i = 0; i < varRefs.size(); i++) {
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

static inline Datatype evaluateDatatype(string expression, uint32_t lineN, vector<string> inputNames, vector<string> cnodeNames) {
	verifyLen(expression, 2, lineN); // Make sure it's not empty

	//TODO add cluster resolution and splitting here

	size_t requiredLength = 3;
	Datatype baseType(BOOL);

	if (expression.find("node(") == 0 && expression.find(")") != expression.npos) {
		requiredLength = expression.find(")") + 1;
		string cnName = trim(expression.substr(5, requiredLength-6));
		bool found = false;
		for (unsigned int i = 0; i < cnodeNames.size(); i++) {
			if (cnodeNames[i] == cnName) {
				baseType = Datatype(i, true); // node(n) constructor
				found = true;
			}
		}
		parse_validate(found, lineN, "Configuration node " + cnName + " could not be found in inputs");
	} else if (expression.find("typeof(") == 0 && expression.find(")") != expression.npos) {
		requiredLength = expression.find(")") + 1;
		string varName = trim(expression.substr(7, requiredLength-8));
		bool found = false;
		for (unsigned int i = 0; i < inputNames.size(); i++) {
			if (inputNames[i] == varName) {
				baseType = Datatype(i, false); // typeof(t) constructor
				found = true;
			}
		}
		parse_validate(found, lineN, "Parameter " + varName + " could not be found in inputs");
	} else if (expression.find("strongestof(") == 0 && expression.find(")") != expression.npos) {
		vector<uint32_t> indices;
		requiredLength = expression.find(")") + 1;
		string varNames = expression.substr(12, requiredLength-13);
		parse_validate(varNames.find(",") != varNames.npos, lineN, "strongestof() list must contain two or more references");
		bool end = false;
		do {
			//What a nightmare
			size_t index = varNames.find(",");
			if (index == varNames.npos) {
				index = varNames.size();
				end = true;
			}
			string varName = trim(varNames.substr(0, index));
			if (!end) varNames = varNames.substr(index+1, varNames.size()-index);
			bool found = false;
			for (unsigned int i = 0; i < inputNames.size(); i++) {
				if (inputNames[i] == varName) {
					indices.push_back(i);
					found = true;
				}
			}
			parse_validate(found, lineN, "Parameter " + varName + " could not be found in inputs");
		} while (!end);

		baseType = Datatype(indices);
	} else {
		bool isInt = false;
		unsigned int type = 0;
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
		else {
			parse_validate(false, lineN, "Invalid or unimplemented datatype: " + expression);
		}
		baseType = Datatype(type);
	}
	if (expression.length() == requiredLength) return baseType; // Not an array
	else {
		//TODO Array
		return baseType;
	}
}

static inline Datatype evaluateDatatypeWithoutAbstracts(string expression, uint32_t lineN, string errorMessage) {
	vector<string> emptyVector;
	Datatype dt = evaluateDatatype(expression, lineN, emptyVector, emptyVector);
	parse_validate(!dt.isAbstract(), lineN, errorMessage);
	return dt;
}

#endif /* DATATYPES_H_ */
