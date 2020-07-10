#include <windows.h>
#include "plugin.hpp"

#include "manager.hpp"
#include "types.hpp"
#include "tools.hpp"
#include "dehrust.hpp"
#include "lang.hpp"

extern PluginStartupInfo startupInfo;
extern Options op;

bool Manager::extractFile(int fNum, BYTE* buffer, int size)
{
  int fSize;
  if(size == 0)
    fSize = 256*files[fNum].noSecs;
  else
    fSize = size;

  BYTE* buf = new BYTE[256*files[fNum].noSecs];
  BYTE* ptr = buf;

  BlockHdr block;
  ExtInfo  info;
  setPointer(infos[fNum].offset);
  do
  {
    read(&block, sizeof(BlockHdr));

    if(block.extInfoSize == 18)
      read(&info, sizeof(ExtInfo));
    else
    {
      BYTE extBuf[255];
      read(extBuf, block.extInfoSize);
      if(block.extInfoSize >= 18)
        CopyMemory(&info, extBuf, sizeof(ExtInfo));
    }

    BYTE* pBuf = new BYTE[block.pSize];
    read(pBuf, block.pSize);
    if(info.pCRC != crc16calc(pBuf, block.pSize))
    {
      delete[] buf;
      delete[] pBuf;
      return false;
    }
    if(block.flags & 0x01)
      CopyMemory(ptr, pBuf, block.size);
    else
      dehrust(pBuf, ptr);
    ptr += block.size;

    delete[] pBuf;
  } while(!(block.flags & 0x02));

  CopyMemory(buffer, buf, fSize);
  delete[] buf;
  return true;
}

int Manager::getFiles(PluginPanelItem *panelItem,
                      int noItems,
                      int move,
                      char *destPath,
                      int opMode)
{
  if(noItems == 0) return 1;
  if(*destPath) addEndSlash(destPath);
  
  char msg[100];
  if(noItems == 1)
    wsprintf(msg, getMsg(MDeHrust1), panelItem[0].FindData.cFileName);
  else
    wsprintf(msg, getMsg(MDeHrust2), noItems);

  char historyName[] = "xHRiP_copy_path";
  
  InitDialogItem items[]={
    DI_DOUBLEBOX,3,1,72,11,0,0,0,0, (char*)MDeHrust,
    DI_TEXT,5,2,0,0,0,0,0,0,msg,
    DI_EDIT,5,3,70,3,1,(int)historyName,DIF_HISTORY,0,destPath,
    DI_TEXT,3,4,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_RADIOBUTTON,5,5,0,0,0,0,DIF_GROUP,0,(char*)MHoBeta,
    DI_RADIOBUTTON,5,6,0,0,0,0,0,0,(char*)MSCL,
    DI_TEXT,3,7,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_CHECKBOX,5,8,0,0,0,0,0,0,(char*)MSkipHeaders,
    DI_TEXT,3,9,0,0,0,0,DIF_BOXCOLOR|DIF_SEPARATOR,0,"",
    DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,1,(char*)MDeHrustButton,
    DI_BUTTON,0,10,0,0,0,0,DIF_CENTERGROUP,0,(char*)MCancel
  };
  
  FarDialogItem dialogItems[sizeof(items)/sizeof(items[0])];
  initDialogItems(items, dialogItems, sizeof(items)/sizeof(items[0]));
  
  switch(op.defaultFormat)
  {
    case 1:
              dialogItems[4].Selected = TRUE;
              break;
    case 2:
              dialogItems[5].Selected = TRUE;
              break;
    default:
              if(noItems == 1)
                dialogItems[4].Selected = TRUE;
              else
                dialogItems[5].Selected = TRUE;
  }
  
  bool singleFile = false, skipHeader = false;
  // если надо показать диалог, то покажем
  if((opMode & OPM_SILENT) == 0)
  {
    int askCode = startupInfo.Dialog(startupInfo.ModuleNumber,
                                     -1, -1, 76, 13,
                                     "DeHrust",
                                     dialogItems,
                                     sizeof(dialogItems)/sizeof(dialogItems[0]));
    if(askCode != 9) return -1;
    lstrcpy(destPath, dialogItems[2].Data);
    
    singleFile  = (dialogItems[4].Selected == FALSE);
    skipHeader = dialogItems[7].Selected;
  }
  
  // если пользователь хочет, то создадим каталоги
  if(GetFileAttributes(destPath)==0xFFFFFFFF)
    for(char *c=destPath; *c; c++)
    {
      if(*c!=' ')
      {
        for(; *c; c++)
          if(*c=='\\')
          {
            *c=0;
            CreateDirectory(destPath, NULL);
            *c='\\';
          }
        CreateDirectory(destPath, NULL);
        break;
      }
    }
  if(*destPath && destPath[lstrlen(destPath)-1] != ':') addEndSlash(destPath);
  
  DWORD noBytesWritten;
  DWORD checkSum     = 0;
  
  int buttonStatus = 0; // состояние кнопок overwrite_all/skip_all
  
  HANDLE file;
  char   name[300];
  
  if(singleFile)
  {
    wsprintf(name, "%s%s", destPath, panelItem[0].FindData.cFileName);

    char* ptr = name + lstrlen(name);
    while(*ptr != '.') { --ptr; }
    *ptr = 0;

    if(skipHeader)
      lstrcat(name, ".bin");
    else
      lstrcat(name, ".scl");
    int r = createFile(file, name, buttonStatus);
    if(r == 0 || r == -1) return -1;
    if(!skipHeader) checkSum = writeSCLHeader(file, noItems);
  }
  
  int exitCode = 1;
  
  if(!openHostFile()) return 0;
  
  HANDLE screen = startupInfo.SaveScreen(0,0,-1,-1);

  DWORD offset = noItems*sizeof(FileHdr) + 8 + 1;
  
  for(int iNum = 0; iNum < noItems; ++iNum)
  {
    // определяем номер файла
    int fNum;
    for(fNum = 0; fNum < noFiles; ++fNum)
      if(!lstrcmp(panelItem[iNum].FindData.cFileName, pcFiles[fNum].name)) break;
    
    // просмотр текстовых файлов без заголовка
    if(noItems == 1 && ((opMode & OPM_VIEW) || (opMode & OPM_EDIT)) && pcFiles[fNum].skipHeader) skipHeader = true;
    
    if(!singleFile)
    {
      wsprintf(name, "%s%s", destPath, panelItem[iNum].FindData.cFileName);
      if(skipHeader) lstrcat(name, ".bin");
      int r = createFile(file, name, buttonStatus);
      if(r == -1)
      {
        exitCode = -1;
        break;
      }
      if(r == 0)
      {
        exitCode = 0;
        continue;
      }
      if(!skipHeader)
      {
        HoHdr hdr;
        CopyMemory(hdr.name, files[fNum].name, 8);
        
        hdr.type     = files[fNum].type;
        hdr.start    = files[fNum].start;
        hdr.size     = files[fNum].size;
        hdr.reserved = 0;
        hdr.noSecs   = files[fNum].noSecs;
        hdr.checkSum = calculateCheckSum(hdr);
        WriteFile(file, &hdr, sizeof(HoHdr), &noBytesWritten, NULL);
      }
    }
    else
    {
      if(!skipHeader)
      {
        FileHdr hdr;
        CopyMemory(hdr.name, files[fNum].name, 8);

        hdr.type   = files[fNum].type;
        hdr.start  = files[fNum].start;
        hdr.size   = files[fNum].size;
        hdr.noSecs = files[fNum].noSecs;

        // пишем заголовок
        SetFilePointer(file, iNum*sizeof(FileHdr)+8+1, NULL, FILE_BEGIN);
        WriteFile     (file, &hdr, sizeof(FileHdr), &noBytesWritten, NULL);
        checkSum += calculateCheckSum((BYTE*)&files[fNum], sizeof(FileHdr));
        
        SetFilePointer(file, offset, NULL, FILE_BEGIN);
        offset += 256*files[fNum].noSecs;
      }
    }
    
    char* msgItems[2];
    msgItems[0] = getMsg(MDeHrusting);
    char  shortName[51] = "...";
    msgItems[1] = name;
    if(lstrlen(name) > 47)
    {
      lstrcat(shortName, name + lstrlen(name) - 47);
      msgItems[1] = shortName;
    }

    if((opMode & OPM_SILENT) == 0)
      messageBox(0, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 0);

    // копируем тело
    BYTE* buf = new BYTE[256*files[fNum].noSecs];

    if(!extractFile(fNum, buf, 0))
    {
      char*         msgItems[3];
      msgItems[0] = getMsg(MCRCError);
      msgItems[1] = panelItem[iNum].FindData.cFileName;
      msgItems[2] = getMsg(MOk);
      
      messageBox(FMSG_WARNING, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 1);
    }
    else
      // пометили файл как успешно скопированный
      panelItem[iNum].Flags ^= PPIF_SELECTED;
    
    checkSum += calculateCheckSum(buf, 256*files[fNum].noSecs);
    
    DWORD noBytesWritten;
    if(skipHeader)
      WriteFile(file, buf, fileSize(files[fNum]), &noBytesWritten, NULL);
    else
      WriteFile(file, buf, 256*files[fNum].noSecs, &noBytesWritten, NULL);

    delete[] buf;

    if(!singleFile) CloseHandle(file);
    
  }
  closeHostFile();

  startupInfo.RestoreScreen(screen);

  if(singleFile)
  {
    if(!skipHeader)
      WriteFile(file, &checkSum, sizeof(checkSum), &noBytesWritten, NULL);
    CloseHandle(file);
  }
  return exitCode;
}
