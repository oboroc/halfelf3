#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <windows.h>
#include "types.hpp"
#include "plugin.hpp"

void  errorMessage    (char *msg);
int   messageBox      (unsigned long flags, char **items, int noItems, int noButtons);
char* pointToName     (char *path);
bool  isValidChar     (BYTE ch);
void  addEndSlash     (char *path);
char* trim            (char *str);

int  memcmp (const BYTE* p1, const BYTE* p2, int maxlen);
// сравнение областей памяти без учета регистра
int  memcmpi(const char* p1, const char* p2, int maxlen);

void  makeTrDosName(char* dest, const ZipedFileHdr& hdr, int width);
int   getNoSecs    (int noSecs);

char* getMsg         (int msgId);
void  initDialogItems(InitDialogItem *init, FarDialogItem *item, int noItems);

DWORD calculateCheckSum(BYTE* ptr, WORD size);
WORD  calculateCheckSum(const HoHdr& hdr);

int   createFile(HANDLE& file, char* name, int& action);

WORD  fileSize      (const ZipedFileHdr& hdr);
bool  isValidZippedFile(ZipedFileHdr& zHdr);
DWORD writeSCLHeader(HANDLE file, BYTE no_files);
DWORD crc32(BYTE* buf, int len);

#endif
