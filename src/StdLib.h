#ifndef STDLIB_H_
#define STDLIB_H_

#include <vector>
#include <string>
#include <OthUtil.h>
#include <OthFile.h>
#include <Function.h>
#include <Datatypes.h>
#include <CallParser.cpp>
#include <FunctionBuilder.cpp>

using namespace std;

static OthFile STANDARD_LIB = OthFile();
static void splitFunctionVarsAndAddTags(Function &f);

inline void define_std() {
	STANDARD_LIB.path = "STD_LIB";

	const string NATIVES = R"(
simple sequence [a:anything]:[b:typeof(a)]
simple sequence [c:boolean, a:anything, b:typeof(a)]?:[r:typeof(a)]

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

simple sequence [a:integer,b:typeof(a)]&[c:typeof(a)]
simple sequence [a:integer,b:typeof(a)]|[c:typeof(a)]
simple sequence [a:integer,b:typeof(a)]^[c:typeof(a)]
simple sequence [a:integer]![b:typeof(a)]
	)";

	addLine(1, trim(NATIVES));

	parse();
	assembleFile(STANDARD_LIB, finalCalls);
	for (Function &f : STANDARD_LIB.functionList) {
		splitFunctionVarsAndAddTags(f);
		f.resolved = true;
	}

	STANDARD_LIB.constants.push_back("_false");
	STANDARD_LIB.constant_types.push_back(Datatype(BOOL));
	STANDARD_LIB.constant_values.push_back("0");
	STANDARD_LIB.constant_lines.push_back(0);
	STANDARD_LIB.constants.push_back("_true");
	STANDARD_LIB.constant_types.push_back(Datatype(BOOL));
	STANDARD_LIB.constant_values.push_back("1");
	STANDARD_LIB.constant_lines.push_back(0);

}

#endif
