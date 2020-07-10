#ifndef DETECTOR_HPP
#define DETECTOR_HPP

#include <windows.h>
#include "types.hpp"

struct FormatInfo
{
  BYTE    type;
  int     size;
  int     start;
  int     noSecs;

  BYTE    signature[32];
  WORD    signatureOffset;
  DWORD   signatureMask;
  BYTE    signatureSize;

  WORD    commentOffset;
  BYTE    commentSize;
  
  BYTE    specialChar;
  BYTE    newType;
  bool    skipHeader;
  char*   description;
};

class Detector
{
  public:
    Detector(char* path);
    ~Detector();

    // �����頥� 0xFF �᫨ ⨯ �� ��।����
    BYTE  detect       (const FileHdr& hdr, const BYTE* secs, int size, char* comment);
    
    void  specialChar  (BYTE n, char *pos);
    void  getType      (BYTE n, char *pos);
    bool  getSkipHeader(BYTE n);
    // �����頥� 0, �᫨
    // ᮮ⢥�����騩 ��ࠬ��� �� �����
    char* description  (BYTE n);
  private:
    BYTE       noFormats;
    FormatInfo formats[255];
};

#endif
