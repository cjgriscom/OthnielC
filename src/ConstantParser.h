#ifndef CONSTANTPARSER_H_
#define CONSTANTPARSER_H_

#include <string>
#include <Datatypes.h>
#include <ctype.h>

using namespace std;

// String: ""
// Char:   ''
// Bool:   false
// I32:    0
// F64:    0.0
// Array:  {0,1,2} or {{0,1},{2,3}}
// Cluster: (0, "", false)

inline bool isConstant(string exp) {
	return exp != ">" && exp != "<" && exp != "?" && exp != "^" &&
			(!(isalpha(exp[0]) || exp[0]=='_') || exp == "true" || exp == "false");
}

inline Datatype getConstantType(string exp, int lineN) {
	if (exp=="true" || exp=="false") {
		return Datatype(BOOL);
	} else if (exp[0] == '\"' && exp[exp.size()-1] == '\"') {
		return Datatype(STRING);
	} else if (exp[0] == '\'' && exp[exp.size()-1] == '\'') {
		return Datatype(CHAR);
	} else if (isdigit(exp[0])) { // Starts with a number
		int dotCount = 0;
		for (char c : exp) { // Validate number and count the number of dots.
			if (c == '.') {dotCount++; continue;}
			parse_validate(isdigit(c), lineN, "Expression " + exp + " could not be evaluated as a numeric constant");
		}
		parse_validate(dotCount <= 1, lineN, "Invalid floating-point expression: "+exp);
		if (dotCount == 1) {
			return Datatype(F64); // F64 (double) is the default float type
		} else {
			return Datatype(I32); // I32 is the default integer type
		}
	} else if (exp[0] == '{' || exp[0] == '(') {
		parse_validate(false, lineN, "Arrays/clusters not yet implemented!!"); //TODO implement clusters/arrays
	}

	parse_validate(false, lineN, "Invalid expression: " + exp);

	return Datatype(BOOL); //Dummy return
}

#endif /* CONSTANTPARSER_H_ */
