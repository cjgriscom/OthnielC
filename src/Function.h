#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <ParsedCall.h>
#include <Call.h>
#include <Datatypes.h>
#include <Keywords.h>

using namespace std;

class Function {
public:
	uint8_t runMode;
	uint8_t memoryMode;
	string functionName;

	uint32_t lineN;

	uint8_t nInputs = 0;
	uint8_t nOutputs = 0;

	/* The first nInputs are inputs,
	 * the next nOutputs are outputs
	 * the rest are auxiliary variables
	 */
	vector<string> variables;
	vector<Datatype> variable_types;
	vector<string> variable_defaults; // No default should be set to "". A default value for an input indicates optional.

	vector<string> confNodes;
	vector<uint8_t> confNode_types; // Where uint8_t refers to the constant confNode types in ConfigurationNode.h

	vector<ParsedCall> callList;

	// Resolved data
	bool			 resolved = false;
	vector<Call>     r_callList;
	vector<Datatype> r_inputs;
	vector<Datatype> r_outputs;
	vector<uint8_t>  r_confNodes() {return confNode_types;}
	vector<Datatype> r_aux;

};

#endif
