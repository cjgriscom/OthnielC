#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <vector>
#include <string>
#include <Datatypes.h>
#include <OthUtil.h>

using namespace std;

const uint8_t INVALID      = 0xFF;

//runMode
const uint8_t SEQUENCE     = 0x00;
const uint8_t PARALLEL     = 0x01;

#define rm_kw(code) (code==SEQUENCE ? "sequence" : "parallel") // Keyword
#define rm_ID(str)  (str=="sequence" ? SEQUENCE : (str=="parallel" ? PARALLEL : INVALID) )

//memoryMode
const uint8_t STATIC       = 0x00;
const uint8_t INLINE       = 0x01;
const uint8_t INSTANTIATED = 0x02;

#define mm_kw(code) (code==STATIC ? "static" : (code==INLINE ? "inline" : "instantiated") )
#define mm_ID(str)  (str=="static" ? STATIC : (str=="inline" ? INLINE : (str=="instantiated" ? INSTANTIATED : INVALID) ) )

//allocMode
const uint8_t PREALLOC     = 0x00;
const uint8_t DYNAMIC      = 0x01;

#define am_kw(code) (code == PREALLOC ? "prealloc" : "dynamic")
#define am_ID(str)  (str=="prealloc" ? PREALLOC : (str=="dynamic" ? DYNAMIC : INVALID) )

static bool is_function_kw(string s) {
	return rm_ID(s) != INVALID || mm_ID(s) != INVALID;
}

struct AbstractCall {
	string callID;
	vector<uint16_t> inPipeIDs;
	vector<uint16_t> outPipeIDs;
};

class Function {
public:
	uint8_t runMode;
	uint8_t memoryMode;
	uint8_t allocMode = PREALLOC;
	string functionName;

	uint8_t nInputs = 0;
	uint8_t nOutputs = 0;

	vector<bool> optionals;

	/* The first nInputs are inputs,
	 * the next nOutputs are outputs
	 * Also includes variables
	 *
	 * Anything past 0xFF indicates a typeof index
	 */
	vector<uint16_t> pipeTypes;


	vector<AbstractCall> callList;



};

#endif
