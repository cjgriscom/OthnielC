#ifndef KEYWORDS_H_
#define KEYWORDS_H_

const uint8_t INVALID      = 0xFF;

//Top-level Directives

const uint8_t VARIABLE     = 0x00;
const uint8_t IMPORT       = 0x01;
const uint8_t ALIAS        = 0x02;

#define directive_kw(code) (code==VARIABLE ? "variable" : (code==IMPORT ? "import" : "alias"))
#define directive_ID(str)  (str=="variable" ? VARIABLE : (str=="import" ? IMPORT : (str=="alias" ? ALIAS : INVALID) ) )

static bool is_directive_kw(string s) {
	return directive_ID(s) != INVALID;
}

//runMode
const uint8_t SEQUENCE     = 0x00;
const uint8_t PARALLEL     = 0x01;

#define rm_kw(code) (code==SEQUENCE ? "sequence" : "parallel") // Keyword
#define rm_ID(str)  (str=="sequence" ? SEQUENCE : (str=="parallel" ? PARALLEL : INVALID) )

//memoryMode
const uint8_t STATIC       = 0x00;
const uint8_t INLINE       = 0x01;
const uint8_t INSTANTIATED = 0x02;
const uint8_t SIMPLE       = 0x03;

#define mm_kw(code) (code==STATIC ? "static" : (code==INLINE ? "inline" : (code==SIMPLE ? "simple" : "instantiated")) )
#define mm_ID(str)  (str=="static" ? STATIC : (str=="inline" ? INLINE : (str=="instantiated" ? INSTANTIATED : (str=="simple" ? SIMPLE : INVALID) ) ) )

static bool is_function_kw(string s) {
	return rm_ID(s) != INVALID || mm_ID(s) != INVALID;
}

#endif
