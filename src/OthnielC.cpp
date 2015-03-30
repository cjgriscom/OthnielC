#include <CallParser.cpp>
#include <FunctionBuilder.cpp>
#include <PipeManager.cpp>
#include <OthFile.h>
#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char **argv) {

	ifstream input("test.othsrc");
	int index = 0;

	for (std::string line; getline( input, line );) {
		addLine(++index, line);
	}
	parse();

	OthFile file;
	assembleFile(&file, &finalCalls);
	setPipeCodes(file);
	testFB(&file);

}
