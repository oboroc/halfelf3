#ifndef types_hpp
#define types_hpp
#include <windows.h>

struct ZipedFileHdr
{
  char  name[8];
  char  type;
  WORD  start;
  WORD  size;
  BYTE  noSecs;

  WORD  packedSize;
  DWORD crc;
  BYTE  pMethod;
  BYTE  flags;
};

struct FileHdr
{
  char name[8];
  char type;
  WORD start;
  WORD size;
  BYTE noSecs;
};

struct ExtFileHdr
{
  char  name[13];
  char  comment[100];
  BYTE  type;
  DWORD offset;
  bool  skipHeader;
};

struct HoHdr
{
  char name[8];
  char type;
  WORD start;
  WORD size;
  BYTE reserved;
  BYTE noSecs;
  WORD checkSum;
};

struct Options
{
  bool showExt;
  int  defaultPanelMode;
  int  defaultFormat;
  bool detectFormat;
  char iniFilePath[300];
};

struct InitDialogItem
{
  unsigned char Type;
  unsigned char X1,Y1,X2,Y2;
  unsigned char Focus;
  unsigned int  Selected;
  unsigned int  Flags;
  unsigned char DefaultButton;
  char         *Data;
};

#endif
