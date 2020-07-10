#ifndef DEHRUST_HPP
#define DEHRUST_HPP
#include <windows.h>

WORD dehrust  (BYTE* source, BYTE* destination);
WORD crc16calc(BYTE* adr, WORD len);

#endif
