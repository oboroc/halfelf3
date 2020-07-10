#include <windows.h>
#include "plugin.hpp"

#include "manager.hpp"
#include "types.hpp"
#include "tools.hpp"
#include "lang.hpp"

extern PluginStartupInfo startupInfo;
extern Options           op;

namespace
{
  enum Action { ASK_USER, DELETE_ALL };
  Action userAction;

  bool   keepSilence;
}

void Manager::move(void)
{
  if(noDelFiles == 0 && noDelFolders == 0) return;

  int  fromSec, fromTrk, toSec, toTrk;

  bool delFileFound = false;
  int  noDelSecs = 0;
  int  toFileNum = 0;
  
  if(noDelFiles != 0)
  {
    for(int i = 0; i < noFiles; ++i)
    {
      if(files[i].name[0] == 0x01)
      {
        noDelSecs += files[i].noSecs;
        if(!delFileFound)
        {
          delFileFound = true;
          toTrk = files[i].trk;
          toSec = files[i].sec;
        }
      }
      else
      {
        if(delFileFound)
        {
          fromTrk = files[i].trk;
          fromSec = files[i].sec;
          files[i].trk = toTrk;
          files[i].sec = toSec;
          int noSecs = files[i].noSecs;
          // ��७�ᨬ 䠩�
          for(int j = 0; j < noSecs; ++j)
          {
            BYTE buf[sectorSize];
            read (fromTrk, fromSec, buf);
            write(toTrk,   toSec,   buf);
            
            if(++fromSec == 16) { fromTrk++; fromSec = 0; }
            if(++toSec   == 16) { toTrk++;   toSec = 0; }
          }
          CopyMemory(&files[toFileNum], &files[i], sizeof(FileHdr));
          CopyMemory(&pcFiles[toFileNum], &pcFiles[i], sizeof(ExtFileHdr));
          if(dsOk) fileMap[toFileNum] = fileMap[i];
        }
        ++toFileNum;
      }
    }

    if(dsOk) for(int i = 0; i < noDelFiles; ++i) fileMap[noFiles-noDelFiles+i] = 0;
    noFiles     -= noDelFiles;
    noDelFiles   = 0;
    diskInfo.firstFreeTrk = toTrk;
    diskInfo.firstFreeSec = toSec;
    diskInfo.noFiles      = noFiles;
    diskInfo.noDelFiles   = noDelFiles;
    diskInfo.noFreeSecs  += noDelSecs;
  }
  if(dsOk && noDelFolders != 0)
  {
    int no_folders = noFolders;
    for(int i = 0; i < no_folders; ++i)
    {
      if(folders[i][0] == 0x01)
      {
        if(curFolderNum == i+1)
        {
          curFolderNum = 0; // �᫨ 㤠���� ⥪�騩 ��⠫��
          *curFolder   = 0; // � ���室�� � ��७�
        }
        if(curFolderNum > i+1) --curFolderNum;
        for(int j = 0; j < noFiles; ++j)
          if(fileMap[j] >= i+1) --fileMap[j];
        for(int j = 0; j < noFolders; ++j)
          if(folderMap[j] >= i+1) --folderMap[j];

        CopyMemory(&folders[i], &folders[i+1], 11*(noFolders-i-1));
        CopyMemory(folderMap+i, folderMap+i+1, noFolders-i-1);
        --i;
        --no_folders;
      }
    }
    for(int i = 0; i < noDelFolders; ++i) folderMap[noFolders-noDelFolders+i] = 0;
    noFolders   -= noDelFolders;
    noDelFolders = 0;
  }
}

ExitCode Manager::markFolder(int fNum)
{
  if(userAction == ASK_USER && !keepSilence)
  {
    int noItems = 0; // ��⠥� �᫮ ��������� 䠩��� � ��⠫����
    for(int i = 0; i < noFiles; ++i)
      if(fileMap[i] == fNum) ++noItems;
    for(int i = 0; i < noFolders; ++i)
      if(folderMap[i] == fNum) ++noItems;
    
    if(noItems > 0) // �᫨ ��⠫�� �� ���⮩, � ...
    {
      char *msgItems[7];
      msgItems[0] = getMsg(MDeleteFolder);
      msgItems[1] = getMsg(MDelFolder);
      msgItems[3] = getMsg(MDeleteButton);
      msgItems[4] = getMsg(MAll);
      msgItems[5] = getMsg(MSkip);
      msgItems[6] = getMsg(MCancel);
      
      char folderName[300];
      char temp[300] = "";
      int  parentFolder = fNum;
      while(parentFolder)
      {
        if(temp[0])
          wsprintf(folderName, "%s\\%s", pcFolders[parentFolder-1], temp);
        else
          wsprintf(folderName, "%s", pcFolders[parentFolder-1]);
        lstrcpy(temp, folderName);
        parentFolder = folderMap[parentFolder-1];
      }
      
      char shortName[50];
      msgItems[2] = truncPathStr(shortName, folderName, 40);
      int button = messageBox(FMSG_WARNING, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 4);
      switch(button)
      {
        case -1:
        case 3:
                  return CANCEL;
        case 2:
                  return SKIP;
        case 1:
                  userAction = DELETE_ALL;
      }
    }
  }
  
  for(int i = 0; i < noFiles; ++i)
  {
    if(fileMap[i] == fNum)
    {
      if(files[i].name[0] != 0x01) ++noDelFiles;
      files[i].name[0] = 0x01;
    }
  }
  ExitCode exitCode = OK;
  for(int i = 0; i < noFolders; ++i)
    if(folderMap[i] == fNum)
    {
      ExitCode code = markFolder(i+1);
      if(code != OK) exitCode = code;
      if(code == CANCEL) break;
    }
  if(exitCode == OK)
  {
    if(folders[fNum-1][0] != 0x01) ++noDelFolders;
    folders[fNum-1][0] = 0x01;
  }
  return exitCode;
}

int Manager::deleteFiles(PluginPanelItem *panelItem, int noItems, int opMode)
{
  if(noItems == 0) return TRUE;
  if(fmt->isProtected(img))
  {
    char *msgItems[4];
    msgItems[0] = getMsg(MWarning);
    msgItems[1] = getMsg(MCanNotDel);
    msgItems[2] = getMsg(MWriteProtected);
    msgItems[3] = getMsg(MOk);
    messageBox(FMSG_WARNING, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 1);
    return FALSE;
  }

  // ����室��� ��� ���४⭮�� ��६�饭�� 䠩���
  // �� ����� ��ࠧ�
  readInfo();

  keepSilence = opMode & OPM_SILENT;
  userAction  = ASK_USER;
  
  if(!keepSilence)
  {
    char *msgItems[5];
    msgItems[0] = getMsg(MDelete);
    msgItems[1] = getMsg(MDeleteAsk);
    msgItems[3] = getMsg(MDeleteButton);
    msgItems[4] = getMsg(MCancel);
    char msg[30];
    if(noItems == 1)
      wsprintf(msg, "%s", panelItem[0].FindData.cFileName);
    else
      wsprintf(msg, getMsg(MDelFiles), noItems);
    
    msgItems[2] = msg;
    if(messageBox(0, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 2)!=0)
      return FALSE;
    if(noItems > 1)
    {
      char *msgItems[5];
      msgItems[0] = getMsg(MDeleteFiles);
      msgItems[1] = getMsg(MDeleteAsk);
      msgItems[3] = getMsg(MAll);
      msgItems[4] = getMsg(MCancel);

      char msg[30];
      wsprintf(msg, getMsg(MDelFiles), noItems);
      msgItems[2] = msg;
      if(messageBox(FMSG_WARNING, msgItems, sizeof(msgItems)/sizeof(msgItems[0]), 2)!=0)
        return FALSE;
    }
  }
  if(!openHostFile()) return FALSE;

  bool allOk = true;
  
  for(int i = 0; i < noItems; ++i)
  {
    if(panelItem[i].FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      int j = 0;
      for(; j < noFolders; ++j)
      {
        if(folderMap[j] != curFolderNum) continue;
        if(!lstrcmp(pcFolders[j], panelItem[i].FindData.cFileName)) break;
      }
      ExitCode code = markFolder(j+1);
      if(code != OK) allOk = false;
      if(code == CANCEL) break;
    }
    else
    {
      for(int j = 0; j < noFiles; ++j)
      {
        if(!lstrcmp(pcFiles[j].name, panelItem[i].FindData.cFileName))
        {
          if(files[j].name[0] != 0x01) ++noDelFiles;
          files[j].name[0] = 0x01;
          break;
        }
      }
    }
  }
  if(op.autoMove)
    move();
  else
    diskInfo.noDelFiles = noDelFiles;

  writeInfo();
  closeHostFile();
  op.reread = true;
  startupInfo.Control(this,FCTL_UPDATEANOTHERPANEL, (void *)1);
  op.reread = false;
  return allOk;
}
