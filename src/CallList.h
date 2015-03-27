#ifndef CALLLIST_H_
#define CALLLIST_H_

#include <OthUtil.h>
#include <Datatypes.h>
using namespace std;

// Numeric operations (work with all numerics and autocast)
const uint8_t std_add = 0x00;
const uint8_t std_sub = 0x01;
const uint8_t std_mul = 0x02;
const uint8_t std_div = 0x03;
const uint8_t std_mod = 0x04;

// Bitwise / boolean operations
const uint8_t std_shl = 0x05; // bitwise shift left
const uint8_t std_shr = 0x06; // bitwise shift right
const uint8_t std_not = 0x07; // !
const uint8_t std_and = 0x08; // &
const uint8_t std_or  = 0x09; // |
const uint8_t std_xor = 0x0A; // ^
const uint8_t std_nand= 0x0B; // !&
const uint8_t std_nor = 0x0C; // !|
const uint8_t std_xnor= 0x0D; // !^

// Assignment
const uint8_t std_asn = 0x0E; // Regular assignment :=
const uint8_t std_ter = 0x0F; // Ternary assignment ?:

// Comparison
const uint8_t std_equ = 0x10; // ==
const uint8_t std_ineq= 0x11; // !=
const uint8_t std_ls  = 0x12; // <
const uint8_t std_gr  = 0x13; // >
const uint8_t std_lse = 0x14; // <=
const uint8_t std_gre = 0x15; // >=

// Casts 0x20 - 0x2F
inline uint8_t std_cast(uint8_t datatype) {
	return 0x20 + datatype; // i.e. (std_cast + F32) = 0x28 or cast to float
}

const uint8_t native  = 0xFE; // extended functions in the native set TODO?
const uint8_t custom  = 0xFF; // user-defined structure

#endif /* CALLLIST_H_ */
