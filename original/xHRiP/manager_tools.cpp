#include <windows.h>

#include "manager.hpp"
#include "detector.hpp"
#include "tools.hpp"

extern Detector* detector;
extern Options   op;

bool Manager::openHostFile(void)
{
  hostFile = CreateFile(hostFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);
  volumeNum = 0;
  return (hostFile != INVALID_HANDLE_VALUE);
}

void Manager::closeHostFile(void)
{
  if(hostFile != INVALID_HANDLE_VALUE) CloseHandle(hostFile);
}

void Manager::makePCNames(void)
{
  for(int fNum = 0; fNum < noFiles; ++fNum)
  {
    int first = 0, last = 7;
    while(first <= last)
    {
      if(isValidChar(files[fNum].name[first])) break;
      ++first;
    }
    while(last >= first)
    {
      if(isValidChar(files[fNum].name[last])) break;
      --last;
    }
    BYTE  noChars = 0;
    BYTE* from    = files  [fNum].name;
    BYTE* to      = pcFiles[fNum].name;
    
    if(first > last)
    {
      FillMemory(to, 8, '_');
      noChars = 8;
    }
    else
    {
      while(first <= last)
      {
        BYTE ch = from[first++];
        to[noChars++] = isValidChar(ch) ? ch : '_';
      }
    }

    // обрабатываем специальные имена устройств
    if(noChars == 3 || noChars == 4)
    {
      if(!memcmpi(to, "com", 3) ||
         !memcmpi(to, "lpt", 3) ||
         !memcmpi(to, "prn", 3) ||
         !memcmpi(to, "con", 3) ||
         !memcmpi(to, "aux", 3) ||
         !memcmpi(to, "nul", 3)) to[noChars++] = '_';
    }

    BYTE dotPos = noChars;
    to[noChars++] = '.';
    to[noChars++] = '$';
    if(isValidChar(files[fNum].type)) to[noChars++] = files[fNum].type;
    to[noChars] = 0;
    
    BYTE typeNum = 0xFF;
    if(op.detectFormat)
    {
      BYTE secs[16*256];
      ZeroMemory(secs, 16*256);

      int noSecs = files[fNum].noSecs;
      if(noSecs > 16) noSecs = 16;
      if(!noSecs)
        if(!extractFile(fNum, secs, noSecs*256)) ZeroMemory(secs, noSecs*256);

      typeNum = detector->detect(files[fNum], secs, noSecs*256, pcFiles[fNum].comment);
    }
    pcFiles[fNum].type = typeNum;

    pcFiles[fNum].skipHeader = false;

    if(typeNum != 0xFF)
    {
      detector->specialChar(typeNum, to+dotPos+1);
      detector->getType(typeNum, to+dotPos+2);
      pcFiles[fNum].skipHeader = detector->getSkipHeader(typeNum);
    }
    // обрабатываем многотомные zxzip архивы
    if(fNum != 0 &&
       !memcmp(from, "********ZIP", 11) &&
       !memcmp(&files[fNum-1].type, "ZIP", 3))
    {
      if(memcmp(files[fNum-1].name, "********", 8))
        lstrcpy(to, pcFiles[fNum-1].name);
      else
        lstrcpyn(to, pcFiles[fNum-1].name, lstrlen(pcFiles[fNum-1].name));
    }
    // обрабатываем повторяющиеся имена
    int i = fNum;
    while(i-- > 0)
    {
      int len = lstrlen(to);
      if(!memcmpi(to, pcFiles[i].name, len))
      {
        BYTE ch = pcFiles[i].name[len];
        ch = (ch != 0) ? ch+1 : '0';
        
        // вот такая кривая обработка :(
        if(ch == '9'+1) ch = 'A'; 
        if(ch == 'Z'+1) ch = 'a';
        if(ch == 'z'+1) ch = '0';
        
        to[len] = ch; to[len+1] = 0;
        break;
      }
    }
  }
}
