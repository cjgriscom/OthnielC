#ifndef STDLIB_H_
#define STDLIB_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <OthFile.h>
#include <Function.h>
#include <CallParser.cpp>
#include <FunctionBuilder.cpp>

using namespace std;

static OthFile STANDARD_LIB = OthFile();
static void splitFunctionVars(Function &f);

inline void define_std() {
	STANDARD_LIB.path = "STD_LIB";

	const string NATIVES = R"(
simple sequence [a:anything]:[b:typeof(a)]

simple sequence [a:numeric]as{type:DATATYPE}[b:node(type)]

simple sequence [a:numeric,b:numeric]+[c:strongestof(a,b)]
simple sequence [a:numeric,b:numeric]-[c:strongestof(a,b)]
simple sequence [a:numeric,b:numeric]*[c:strongestof(a,b)]
simple sequence [a:numeric,b:numeric]/[c:strongestof(a,b)]
simple sequence [a:numeric,b:numeric]%[c:strongestof(a,b)]

simple sequence [a:numeric,b:numeric]>[c:boolean]
simple sequence [a:numeric,b:numeric]>=[c:boolean]
simple sequence [a:numeric,b:numeric]<[c:boolean]
simple sequence [a:numeric,b:numeric]<=[c:boolean]
simple sequence [a:numeric,b:numeric]=[c:boolean]

simple sequence [a:boolean,b:boolean]&[c:boolean]
simple sequence [a:boolean,b:boolean]|[c:boolean]
simple sequence [a:boolean,b:boolean]^[c:boolean]
simple sequence [a:boolean]![b:boolean]
	)";

	int index = 0;
	while (index == 0){
		addLine(++index, trim(NATIVES));
	}

	parse();
	assembleFile(STANDARD_LIB, finalCalls);
	for (Function &f : STANDARD_LIB.functionList) {
		splitFunctionVars(f);
		f.resolved = true;
	}

}


#endif
