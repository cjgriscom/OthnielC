#ifndef CALLPARSER_CPP_
#define CALLPARSER_CPP_

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <OthUtil.h>
#include <ParsedCall.h>
using namespace std;

const uint32_t PARAMETER = 1;
const uint32_t SPACE     = 2;
const uint32_t CALL_NAME = 4;
const uint32_t CONF_NODE = 8;
const uint32_t COLON     = 16;

static vector<ParsedCall> calls;

class Code {
	string contents = "";
	map<int32_t, int32_t> lineNMap;
public:
	void addLine(int32_t lineN, string line) {
		lineNMap.insert(std::pair<int32_t,int32_t>(contents.length(), lineN));
		if (contents.length() != 0) contents += " ";
		contents = contents + trim(line);
	}

	int32_t lineNOfIndex(int32_t index) {
		std::map<int,int>::iterator lineBase = lineNMap.lower_bound(index);
		return lineBase == lineNMap.end() ? -1 : lineBase->second;
	}

	int32_t trueIndex(int32_t index) { // TODO deal with whitespace
		std::map<int,int>::iterator lineBase = lineNMap.lower_bound(index);
		return lineBase == lineNMap.end() ? -1 : index - lineBase->second;
	}

	int32_t length() {
		return contents.size();
	}

	char at(int32_t index) {
		return contents.at(index);
	}

	int32_t indexOf(string seq) {
		int32_t pos = contents.find(seq);
		return pos == seq.npos ? -1 : pos;
	}

	string substr_len(int32_t paramInt1, int32_t paramInt2) {
		return contents.substr(paramInt1, paramInt2-paramInt1);
	}

	void clear() {
		contents = "";
		lineNMap.clear();
	}
};

// Collection of space-delimited concatenated lines of code
static Code lines;

struct Component {
	uint32_t type;
	uint32_t beginIndex;
	uint32_t lineN;
	string content;
};
static Component getComponent(uint32_t type, string content, uint32_t beginIndex,
		uint32_t lineN) {
	Component c;
	c.type = type;
	c.content = trim(content);
	c.beginIndex = beginIndex;
	c.lineN = lineN;
	return c;
}
static Component getSpace(uint32_t beginIndex, uint32_t lineN) {
	Component c;
	c.type = SPACE;
	c.beginIndex = beginIndex;
	c.lineN = lineN;
	return c;
}

static void addLine(int32_t lineN, string line) {
	int32_t commentStart = line.find("//");
	if (commentStart != line.npos) line = line.substr(0, commentStart); // Strip comments from line
	if (trim(line) == "") return;
	lines.addLine(lineN, line);
}

static void removeFirstN(int32_t num) {
	calls.erase(calls.begin()+0, calls.begin()+num);
}

static ParsedCall get(uint32_t index) {
	return calls[index];
}

static ParsedCall firstCall() {
	return get(0);
}

static int32_t size() {
	return calls.size();
}

static vector<ParsedCall> getCalls() {
	return calls;
}

static bool isEmpty() {
	return lines.length() == 0;
}

static vector<string> adjoinedInsFromOuts(vector<string> outs) {
	// Determine length, scrapping garbage pipes
	int32_t length = 0;
	for (string label : outs) { // TODO maybe since I'm using vector this isn't needed
		if (label != "^") {
			length++;
		}
	}

	vector<string> ins(length); // Add each of the non-garbage labels to the clone
	int32_t i = 0;
	for (string label : outs) {
		if (label == ">") {
			ins[i] = "<";
			i++;
		} else if (label != "^") {
			ins[i] = label;
			i++;
		}
	}
	return ins;
}

static ParsedCall advance(ParsedCall currentCall, Component c) {
	parse_validate(currentCall.callName != "UNDEFINED", c.lineN,
			"Floating parameters at index",  c.beginIndex);
	calls.push_back(currentCall);
	ParsedCall newCall;
	return newCall;
}

static void processIntoCalls(vector<Component> components) {
	uint32_t expected = PARAMETER + CALL_NAME + COLON;
	ParsedCall currentCall;
	uint32_t lastType = SPACE; // Coming off of another line, theoretically a space exists in between
	uint32_t type = SPACE;
	Component c;
	for (uint32_t i = 0; i < components.size(); i++) {
		c = components[i];
		currentCall.lineN = c.lineN;
		verifyExpectedBit(expected, c.type, c.lineN,
				"Unexpected thingy at index",  c.beginIndex);
		lastType = type;
		type = c.type;
		if (type == PARAMETER) {

			int32_t nParams = 0;
			i--;
			while (components.size() > i + 1
					&& components[i + 1].type == PARAMETER) {
				i++;
				if (components[i].content.size() != 0)
					nParams++; // An empty param denotes empty brackets; we don't want to add them
			}
			vector<string> paramArray(nParams);
			for (int32_t j = 0; j < nParams; j++) {
				paramArray[j] = components[i - nParams + j + 1].content;
			}
			// Done processing params
			if (lastType == CALL_NAME || lastType == CONF_NODE) { // I.E. this is an out parameter
				currentCall.outParams = paramArray;
				expected = CALL_NAME + SPACE + COLON;
			} else {  // I.E. this is an in parameter
				currentCall.inParams = paramArray;
				expected = CALL_NAME;
			}
		} else if (c.type == CALL_NAME) {
			if (currentCall.outParams.size() != 0) { // Adjoined call
				// Determine adjoined inputs and advance currentCall to new one
				vector<string> newIns = adjoinedInsFromOuts(currentCall.outParams);
				currentCall = advance(currentCall, c);
				currentCall.inParams = newIns;
			}
			currentCall.callName = c.content;
			expected = CONF_NODE + SPACE + PARAMETER + COLON;
		} else if (c.type == CONF_NODE) {
			expected = SPACE + PARAMETER + COLON;
			int32_t nNodes = 0;
			i--;
			while (components.size() > i + 1
					&& components[i + 1].type == CONF_NODE) {
				i++;
				if (components[i].content.size() != 0)
					nNodes++; // An empty param denotes empty brackets; we don't want to add them
			}
			vector<string> cnodes(nNodes);
			for (int32_t j = 0; j < nNodes; j++) {
				cnodes[j] = components[i - nNodes + j + 1].content;
			}
			currentCall.confNodes = cnodes;
		} else if (c.type == SPACE) {
			// Advance currentCall to new one
			currentCall = advance(currentCall, c);
			expected = PARAMETER + CALL_NAME + COLON;
		} else if (c.type == COLON) {
			if (lastType == SPACE) {
				currentCall.isBlockEnd = true;
				expected = CALL_NAME + PARAMETER;
			} else {
				currentCall.isBlockStart = true;
				expected = SPACE;
			}
		}
	}
	advance(currentCall, c);
}

static vector<Component> separateComponents() {
	// Deconstruct line byte by byte

	vector<Component> components;

	// TODO exclude things like "]["
	int32_t btbbrackets = lines.indexOf("][");
	if (btbbrackets == -1)
		btbbrackets = lines.indexOf("}{");
	parse_validate(btbbrackets == -1, lines.lineNOfIndex(btbbrackets),
			"Encountered illegal brackets at index", btbbrackets);

	uint32_t NOTHING = 0;

	int32_t beginIndex = 0;

	bool inQuotes = false;
	uint32_t curlyCount = 0;
	uint32_t current = NOTHING;
	char c;
	bool atEnd;

	for (int32_t i = 0; i <= lines.length(); i++) {
		atEnd = i + 1 == lines.length(); // for colon checking
		if (i == lines.length()) {
			parse_validate(current == NOTHING || current == CALL_NAME,
					lines.lineNOfIndex(i), "Expected closing bracket");
			if (current == CALL_NAME) {
				components.push_back(
						getComponent(current, lines.substr_len(beginIndex, i),
								lines.trueIndex(beginIndex),
								lines.lineNOfIndex(i)));
			}
			break;
		} else {
			c = lines.at(i);
		}

		if (c <= ' ' && curlyCount == 0) { // Process space or tab IF
			if (current != PARAMETER && // Not a parameter list
					current != CONF_NODE && // Not a list of conf nodes
					(components.size() == 0
							|| // At beginning of list
							components[components.size() - 1].type != SPACE
							||  // or Last one wasn't a space
							current == CALL_NAME) // or we're in a call name
					) {
				if (current == CALL_NAME) {
					components.push_back(
							getComponent(current,
									lines.substr_len(beginIndex, i),
									lines.trueIndex(beginIndex),
									lines.lineNOfIndex(i)));
					current = NOTHING;
				}
				components.push_back(
						getSpace(lines.trueIndex(i),
								lines.lineNOfIndex(i))); // Add spaces between calls
			}
		} else if (inQuotes && curlyCount == 0) { // We want to ignore other symbols when in quotation marks
			if (c == '"' || c == '\'') {
				inQuotes = false; // Encountered end
			}
		} else if ((c == '"' || c == '\'') && curlyCount == 0) { // If quotes found
			parse_validate(current == PARAMETER || current == CONF_NODE,
					lines.lineNOfIndex(i),
					"Encountered unexpected \" at index",  lines.trueIndex(i));
			inQuotes = true;
		} else if (c == '[') { // Open params
			if (curlyCount == 0) {
				parse_validate(current == NOTHING || current == CALL_NAME,
						lines.lineNOfIndex(i),
						"Encountered unexpected [ at index "
								, lines.trueIndex(i));
				if (current == CALL_NAME)  // End of call
					components.push_back(
							getComponent(current,
									lines.substr_len(beginIndex, i),
									lines.trueIndex(beginIndex),
									lines.lineNOfIndex(i)));
				current = PARAMETER;
				beginIndex = i + 1;
			} else {
				curlyCount++;
			}
		} else if (c == '{') { // Open conf nodes
			if (curlyCount == 0) {
				parse_validate(current == NOTHING || current == CALL_NAME,
						lines.lineNOfIndex(i),
						"Encountered unexpected { at index "
								, lines.trueIndex(i));
				if (current == CALL_NAME) // End of call
					components.push_back(
							getComponent(current,
									lines.substr_len(beginIndex, i),
									lines.trueIndex(beginIndex),
									lines.lineNOfIndex(i)));
				current = CONF_NODE;
				beginIndex = i + 1;
			}
			curlyCount++;

		} else if (c == ']') {
			if (curlyCount == 0) {
				parse_validate(current == PARAMETER, lines.lineNOfIndex(i),
						"Encountered unexpected ] at index "
								, lines.trueIndex(i));
				string item = lines.substr_len(beginIndex, i);
				components.push_back(
						getComponent(current, item,
								lines.trueIndex(beginIndex),
								lines.lineNOfIndex(i)));
				current = NOTHING;
			} else {
				curlyCount--;
			}
		} else if (c == '}') {
			if (curlyCount <= 1) {
				parse_validate(current == CONF_NODE, lines.lineNOfIndex(i),
						"Encountered unexpected } at index "
								, lines.trueIndex(i));
				string item = lines.substr_len(beginIndex, i);
				components.push_back(
						getComponent(current, item,
								lines.trueIndex(beginIndex),
								lines.lineNOfIndex(i)));
				current = NOTHING;
			}
			curlyCount--;
		} else if (c == ',' && curlyCount <= 1) {
			parse_validate(current == PARAMETER || current == CONF_NODE,
					lines.lineNOfIndex(i),
					"Encountered unexpected , at index",  lines.trueIndex(i));
			components.push_back(
					getComponent(current, lines.substr_len(beginIndex, i),
							lines.trueIndex(beginIndex),
							lines.lineNOfIndex(i)));
			beginIndex = i + 1;
		} else if (c == ':' && (current == NOTHING || current == CALL_NAME)
				&& ((atEnd || lines.at(i + 1) <= ' ') || // Make sure it's not a : or ?: native
						(i == 0 || lines.at(i - 1) <= ' '))) {
			if (current == CALL_NAME) { // End of call
				components.push_back(
						getComponent(current, lines.substr_len(beginIndex, i),
								lines.trueIndex(beginIndex),
								lines.lineNOfIndex(i)));
				current = NOTHING;
			}
			components.push_back(
					getComponent(COLON, "", lines.trueIndex(i),
							lines.lineNOfIndex(i)));
		} else {
			if (current == NOTHING) {
				current = CALL_NAME;
				beginIndex = i;
			}
		}
	}
	return components;
}

static void parse() {
	//Clear calls
	calls.clear();

	vector<Component> components = separateComponents();
	processIntoCalls(components);

	//Clear old data
	lines.clear();
}

static void test() {
	int indent = 0;

	for (ParsedCall call : calls) {
		if (call.isBlockEnd) {
			indent--;
		}
		for (int ind = 0; ind < indent; ind++) {
			cout << "  ";
		}
		if (call.isBlockEnd)
			cout << ":";
		cout << "[";
		for (string param : call.inParams) {
			cout << param << " ";
		}
		cout << "]";
		cout << call.callName;
		if (call.confNodes.size() > 0) {
			cout << "{ ";
			for (string param : call.confNodes) {
				cout << param << " ";
			}
			cout << "}";
		}

		cout << "[";
		for (string param : call.outParams) {
			cout << param << " ";
		}
		cout << "]";
		if (call.isBlockStart) {
			indent++;
			cout << ":";
		}
		cout << endl;
	}
}

#endif
