#include <windows.h>
#include "plugin.hpp"

#include "tools.hpp"
#include "types.hpp"
#include "lang.hpp"

extern PluginStartupInfo startupInfo;
extern Options           op;

void errorMessage(char *msg)
{
  char *msgItems[3];
  msgItems[0] = getMsg(MError);
  msgItems[1] = msg;
  msgItems[2] = getMsg(MOk);
  startupInfo.Message(startupInfo.ModuleNumber,
                      FMSG_WARNING, NULL,
                      msgItems,
                      sizeof(msgItems)/sizeof(msgItems[0]), 1);
}

int messageBox(unsigned long flags, char **items, int noItems, int noButtons)
{
  return (startupInfo.Message(startupInfo.ModuleNumber,
                              flags, NULL, items,
                              noItems, noButtons));
}

char* pointToName(char *path)
{
  char *namePtr = path;
  while(*path)
  {
    if(*path == '\\' || *path == '/' || *path==':') namePtr = path+1;
    path++;
  }
  return namePtr;
}

bool isValidChar(BYTE ch)
{
  if(ch < 0x20 || ch > 0x7F) return false;
  BYTE invalidChar[] = {' ', '.', '\\', '/', ':', '?', '*', '<', '>', '|'};
  for(int i = 0; i < sizeof(invalidChar); i++)
    if(ch == invalidChar[i]) return false;
  return true;
}

void addEndSlash(char *path)
{
  int length=lstrlen(path);
  if(length == 0 || path[length-1] != '\\') lstrcat(path,"\\");
}

char* trim(char *str)
{
  int i,j,len;

  len = lstrlen(str);

  for(i = len-1; i>=0; i--)
    if(str[i] != ' ') break;
  str[i+1] = 0;

  for(i = 0; i < len; i++)
    if(str[i] != ' ') break;
  if(i == 0) return str;

  for(j = i; j < len; j++)
    str[j-i] = str[j];

  return str;
}

int memcmp(const BYTE* p1, const BYTE* p2, int maxlen)
{
  if(!maxlen) return(0);
  while(maxlen-- && *p1 == *p2) { p1++; p2++; }
  return(*p1 - *p2);
}

int memcmpi(const char* p1, const char* p2, int maxlen)
{
  if(!maxlen) return(0);
  char ch1, ch2;
  
  while(maxlen--)
  {
    ch1 = (char)CharUpper((char*)*p1++);
    ch2 = (char)CharUpper((char*)*p2++);
    if(ch1 != ch2) break;
  }
  return (ch1 - ch2);
}

bool isEnAlphaNum(BYTE ch)
{
  if(ch >= '0' && ch <= '9') return true;
  if(ch >= 'A' && ch <= 'Z') return true;
  if(ch >= 'a' && ch <= 'z') return true;
  return false;
}

void makeTrDosName(char* dest, const FileHdr& hdr, int width)
{
  for(int i = 0; i < 8; ++i) dest[i] = hdr.name[i] ? hdr.name[i] : ' ';
  
  BYTE ch1, ch2, ch3;
  ch1 = hdr.type;
  ch2 = hdr.start & 0xff;
  ch3 = hdr.start>>8;
  
  if(op.showExt &&
     isEnAlphaNum(ch1) &&
     isEnAlphaNum(ch2) &&
     isEnAlphaNum(ch3))
  {
    dest[width-3] = ch1;
    dest[width-2] = ch2;
    dest[width-1] = ch3;
  }
  else
  {
    dest[width-3] = '<';
    dest[width-2] = ch1 ? ch1 : ' ';
    dest[width-1] = '>';
  }
  dest[width] = 0;
}

char* getMsg(int msgId)
{
  return(startupInfo.GetMsg(startupInfo.ModuleNumber, msgId));
}

void initDialogItems(InitDialogItem *init, FarDialogItem *item, int noItems)
{
  for(int i = 0; i < noItems; i++)
  {
    item[i].Type          = init[i].Type;
    item[i].X1            = init[i].X1;
    item[i].Y1            = init[i].Y1;
    item[i].X2            = init[i].X2;
    item[i].Y2            = init[i].Y2;
    item[i].Focus         = init[i].Focus;
    item[i].Selected      = init[i].Selected;
    item[i].Flags         = init[i].Flags;
    item[i].DefaultButton = init[i].DefaultButton;
    if((unsigned int)init[i].Data < 2000)
      lstrcpy(item[i].Data, getMsg((unsigned int)init[i].Data));
    else
      lstrcpy(item[i].Data, init[i].Data);
  }
}

DWORD calculateCheckSum(BYTE* ptr, WORD size)
{
  DWORD sum = 0;
  while(size--) sum += *ptr++;
  return sum;
}

WORD calculateCheckSum(const HoHdr& hdr)
{
  WORD sum = 0;
  BYTE* p = (BYTE*)&hdr;
  for(int i = 0; i < sizeof(HoHdr)-2; ++i) sum += *p++;
  return 257*sum + 105;
}

int createFile(HANDLE& file, char* name, int& action)
{
  // проверяем наличие файла
  WIN32_FIND_DATA data;
  HANDLE h = FindFirstFile(name, &data);
  if(h != INVALID_HANDLE_VALUE)
  {
    // файл с таким именем существует
    FindClose(h);
    if(action == 2) return 0;
    if(action == 0)
    {
      char *msgItems[11];
      msgItems[0]  = getMsg(MWarning);
      msgItems[1]  = getMsg(MAlreadyExists);
      msgItems[2]  = name;
      msgItems[3]  = "\x001";
      msgItems[4]  = "";
      msgItems[5]  = "\x001";
      msgItems[6]  = getMsg(MOverwrite);
      msgItems[7]  = getMsg(MOverwriteAll);
      msgItems[8]  = getMsg(MSkip);
      msgItems[9]  = getMsg(MSkipAll);
      msgItems[10] = getMsg(MCancel);

      char param[50];
      if(data.ftLastWriteTime.dwLowDateTime == 0 &&
         data.ftLastWriteTime.dwHighDateTime == 0)
      {
        wsprintf(param, getMsg(MDest1), data.nFileSizeLow);
      }
      else
      {
        FILETIME lastWriteTime;
        FileTimeToLocalFileTime(&data.ftLastWriteTime, &lastWriteTime);
        
        SYSTEMTIME time;
        FileTimeToSystemTime(&lastWriteTime, &time);
        
        wsprintf(param, getMsg(MDest2),
                 data.nFileSizeLow,
                 time.wDay, time.wMonth, time.wYear,
                 time.wHour, time.wMinute, time.wSecond);
      }
      msgItems[4] = param;
      int askCode = messageBox(FMSG_WARNING | FMSG_DOWN,
                               msgItems,
                               sizeof(msgItems)/sizeof(msgItems[0]), 5);
      switch(askCode)
      {
        case -1:
        case 4:
                  return -1;
        case 3:
                  action = 2; //skipAll
        case 2:
                  return 0;
        case 1:
                  action = 1; //overwriteAll
      }
    }
  }
  file = CreateFile(name,
                    GENERIC_WRITE,
                    FILE_SHARE_READ,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL);
  if(file != INVALID_HANDLE_VALUE) return 1;
  
  char *msgItems[4];
  msgItems[0] = getMsg(MError);
  msgItems[1] = getMsg(MCanNotCreate);
  msgItems[2] = name;
  msgItems[3] = getMsg(MOk);
  messageBox(FMSG_WARNING, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 1);
  return 0;
}

WORD fileSize(const FileHdr& hdr)
{
  WORD size;
  if(hdr.type == 'b' || hdr.type == 'B')
    size = 4 + hdr.start;
  else
    size = hdr.size;
  WORD noSecs = size/256 + (size%256 ? 1 : 0);
  if(noSecs != hdr.noSecs) size = 256*hdr.noSecs;
  return size;
}

DWORD writeSCLHeader(HANDLE file, BYTE no_files)
{
  BYTE  signature[] = {'S', 'I', 'N', 'C', 'L', 'A', 'I', 'R' };
  DWORD noBytesWritten;
  
  WriteFile(file, signature, sizeof(signature), &noBytesWritten, NULL);
  WriteFile(file, &no_files, 1, &noBytesWritten, NULL);
  
  // резервируем место для каталога
  SetFilePointer(file, no_files*sizeof(FileHdr)+8+1, NULL, FILE_BEGIN);
  SetEndOfFile(file);
  
  return (0x255 + no_files);
}
