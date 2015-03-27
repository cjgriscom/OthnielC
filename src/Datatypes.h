#ifndef DATATYPES_H_
#define DATATYPES_H_

using namespace std;

//Datatypes
const uint8_t U8  = 0x00;
const uint8_t I8  = 0x01;
const uint8_t U16 = 0x02;
const uint8_t I16 = 0x03;
const uint8_t U32 = 0x04;
const uint8_t I32 = 0x05;
const uint8_t U64 = 0x06;
const uint8_t I64 = 0x07;

const uint8_t F32 = 0x08; // Float
const uint8_t F64 = 0x09; // Double
const uint8_t F80 = 0x0A; // Long double

const uint8_t C32 = 0x0C; // Complex
const uint8_t C64 = 0x0D;
const uint8_t C80 = 0x0E;

const uint8_t BOOL     = 0x10;
const uint8_t STRING   = 0x11;

const uint8_t REFERENCE= 0xF0; //TODO not totally sure how this works

// Abstract types
const uint8_t INTEGER  = 0xFD;
const uint8_t NUMERIC  = 0xFE;
const uint8_t ANYTHING = 0xFF;

#endif
