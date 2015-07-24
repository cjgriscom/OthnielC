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

	string toString_name_confNodes() {
		string str = "";
		str += functionName;
		if (confNodes.size() > 0) {
			str += "{";
			for (unsigned int i = 0; i < confNodes.size(); i++) {
				str += confNodes[i] + ":" + cn_kw(confNode_types[i]);
				if (i < confNodes.size()-1) str += ", ";
			}
			str +=  "}";
		}
		return str;
	}

	string toString() {
		string str = "";
		bool hasAux = false;
		if (variables.size() == 0) str += toString_name_confNodes();
		for (int i = 0; i < ((int)variables.size()); i++) {
			if (nInputs == 0) str += toString_name_confNodes();
			if ((i == 0 && nInputs > 0) || (i == nInputs && nOutputs > 0)) str += "[";
			if (i == nInputs+nOutputs) {str += " <"; hasAux=true;}
			str += variables[i] + ":" + variable_types[i].asString();
			if (variable_defaults[i].size() > 0) str += "=" + variable_defaults[i];
			bool printAuxBracket = i == ((int)variables.size())-1 && hasAux;
			if (i == nInputs-1 || i == nInputs+nOutputs-1) str += "]";
			else if (!printAuxBracket) str += ", ";
			if (i == nInputs-1) str += toString_name_confNodes();
			if (printAuxBracket) str += ">";
		}
		hasAux = false;
		return str;
	}

};

static pair<uint8_t,uint32_t> vr_Construct_func(Function * func, uint32_t index, Datatype * type) {
	uint8_t mode;
	uint32_t i;
	if (index < func->nInputs) {
		mode = VAR_IN;
		i = index;

		if (func->r_inputs[i].isIndependantAbstract()) { // References an independent abstract; dependent on call input type
			*type = Datatype(index, false);
		} else {
			*type = func->r_inputs[i];
		}
	} else if (index < func->nInputs + func->nOutputs) {
		mode = VAR_OUT;
		i = index - func->nInputs;
		*type = func->r_outputs[i];
	} else {
		mode = VAR_AUX;
		i = index - func->nInputs - func->nOutputs;
		*type = func->r_aux[i];
	}
	return make_pair(mode, i);
}

#endif
