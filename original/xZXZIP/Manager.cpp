#include <windows.h>
#include "plugin.hpp"

#include "manager.hpp"
#include "detector.hpp"
#include "types.hpp"
#include "tools.hpp"
#include "lang.hpp"

extern PluginStartupInfo startupInfo;
extern Detector*         detector;
extern Options           op;

HANDLE Manager::open(int fNum)
{
  if(fNum >= noVolumes) return INVALID_HANDLE_VALUE;
  
  return CreateFile(volumeName[fNum],
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL);
}

bool Manager::setPointer(DWORD offset)
{
  int vNum = 0;
  for(; vNum < noVolumes; ++vNum)
  {
    if(volumeSize[vNum] < offset)
      offset -= volumeSize[vNum];
    else
      break;
  }
  if(vNum == noVolumes) return false;
  if(vNum == volumeNum)
  {
    SetFilePointer(hostFile, offset + sizeof(HoHdr), NULL, FILE_BEGIN);
    return true;
  }
  CloseHandle(hostFile);

  hostFile = open(vNum);
  if(hostFile == INVALID_HANDLE_VALUE) return false;
  SetFilePointer(hostFile, offset + sizeof(HoHdr), NULL, FILE_BEGIN);
  volumeNum = vNum;
  return true;
}

bool Manager::read(void* buf, WORD size)
{
  DWORD noBytesRead;
  ReadFile(hostFile, buf, size, &noBytesRead, NULL);
  if(noBytesRead == size) return true;
  
  if(++volumeNum == noVolumes) return false;
  
  CloseHandle(hostFile);
  hostFile = open(volumeNum);
  
  if(hostFile == INVALID_HANDLE_VALUE) return false;
  SetFilePointer(hostFile, sizeof(HoHdr), NULL, FILE_BEGIN);
  ReadFile(hostFile, (BYTE*)buf + noBytesRead, size - noBytesRead, &noBytesRead, NULL);

  return true;
}

Manager::Manager(char* fileName)
{
  lstrcpy(hostFileName, fileName);
  noFiles   = 0;
  volumeNum = 0;
  
  ZeroMemory(&lastModifed, sizeof(WIN32_FIND_DATA));

  HANDLE hFile = CreateFile(hostFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL);
  DWORD fileSize = GetFileSize(hFile, NULL);
  CloseHandle(hFile);

  volumeSize  [0] = fileSize - sizeof(HoHdr);
  volumeName  [0] = new char[300];
  lstrcpy(volumeName[0], hostFileName);

  char name[300];
  lstrcpy(name, hostFileName);
  int len = lstrlen(name);
  if(name[len-1] != '0')
  {
    len++;
    lstrcat(name, "0");
  }
  else
    name[len-1]++;

  for(noVolumes = 1; noVolumes < 10; noVolumes++)
  {
    HANDLE volume = CreateFile(name,
                               GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               NULL,
                               OPEN_EXISTING,
                               FILE_FLAG_SEQUENTIAL_SCAN,
                               NULL);
    if(volume == INVALID_HANDLE_VALUE) break;

    HoHdr hdr;
    DWORD noBytesRead;

    ReadFile(volume, &hdr, sizeof(HoHdr), &noBytesRead, NULL);
    fileSize = GetFileSize(volume, NULL);
    CloseHandle(volume);

    if(hdr.checkSum != calculateCheckSum(hdr)) continue;

    if(memcmp(&hdr.name, "********", 8) || memcmp(&hdr.type, "ZIP", 3)) break;
    volumeSize  [noVolumes] = fileSize - sizeof(HoHdr);
    volumeName  [noVolumes] = new char[300];
    lstrcpy(volumeName[noVolumes], name);

    name[len-1]++;
  }
}

Manager::~Manager()
{
  for(int i = 0; i < noVolumes; ++i)
    delete[] volumeName[i];
}

void Manager::getOpenPluginInfo(OpenPluginInfo* info)
{
  info->StructSize    = sizeof(*info);
  info->Flags         = OPIF_USEFILTER | OPIF_USEHIGHLIGHTING | OPIF_ADDDOTS | OPIF_SHOWNAMESONLY | OPIF_SHOWPRESERVECASE;
  info->HostFile      = hostFileName;
  info->CurDir        = "";
  info->Format        = "ZXZIP";
  info->StartSortMode = SM_UNSORTED;
  
  static char panelTitle[260];
  panelTitle[0] = 0;
  lstrcat(panelTitle, " ZXZIP:");
  lstrcat(panelTitle, pointToName(hostFileName));
  lstrcat(panelTitle, " ");
  info->PanelTitle = panelTitle;

  static PanelMode   mode[10];
  static char*       columnTitles3[4];
  columnTitles3[0] = getMsg(MName);
  columnTitles3[1] = getMsg(MSectorSize);
  columnTitles3[2] = getMsg(MName);
  columnTitles3[3] = getMsg(MSectorSize);

  static char*       columnTitles4[6];
  columnTitles4[0] = getMsg(MName);
  columnTitles4[1] = getMsg(MSize);
  columnTitles4[2] = getMsg(MStart);
  columnTitles4[3] = getMsg(MSectorSize);
  columnTitles4[4] = getMsg(MPackedSize);
  columnTitles4[5] = getMsg(MMethod);

  static char*       columnTitles5[2];
  columnTitles5[0] = getMsg(MName);
  columnTitles5[1] = getMsg(MFormat);

  static char*       columnTitles6[2];
  columnTitles6[0] = getMsg(MName);
  columnTitles6[1] = getMsg(MComment);
  
  // C0 - trdos name
  // C1 - trdos 'start'
  // C2 - размер в секторах
  // C3 - packedSize
  // C4 - метод упаковки
  // C5 - описание типа
  // C6 - комментарий к файлу
  
  mode[3].ColumnTypes="N,C2,N,C2";
  mode[3].ColumnWidths="0,3,0,3";
  mode[3].ColumnTitles=columnTitles3;
  mode[3].StatusColumnTypes="C0,S,C2";
  mode[3].StatusColumnWidths="0,5,3";
  mode[3].AlignExtensions=TRUE;
  mode[3].FullScreen=FALSE;
  
  mode[4].ColumnTypes="C0,S,C1,C2,C3,C4";
  mode[4].ColumnWidths="0,5,5,3,5,3";
  mode[4].StatusColumnTypes="N,S,C2";
  mode[4].StatusColumnWidths="0,5,3";
  mode[4].ColumnTitles=columnTitles4;
  mode[4].FullScreen=FALSE;
  
  mode[5].ColumnTypes="C0,C5";
  mode[5].ColumnWidths="12,0";
  mode[5].StatusColumnTypes="N,S,C2";
  mode[5].StatusColumnWidths="0,5,3";
  mode[5].ColumnTitles=columnTitles5;
  mode[5].FullScreen=FALSE;

  mode[6].ColumnTypes="C0,C6";
  mode[6].ColumnWidths="12,0";
  mode[6].StatusColumnTypes="N,S,C2";
  mode[6].StatusColumnWidths="0,5,3";
  mode[6].ColumnTitles=columnTitles6;
  mode[6].FullScreen=FALSE;
  
  info->PanelModesArray = mode;
  info->PanelModesNumber = sizeof(mode)/sizeof(mode[0]);
  info->StartPanelMode = '0' + op.defaultPanelMode;
}

int Manager::getFindData(PluginPanelItem **pPanelItem, int *pNoItems, int opMode)
{
  if(!readInfo()) return FALSE;

  PluginPanelItem *item = new PluginPanelItem[noFiles];
  *pNoItems   = noFiles;
  *pPanelItem = item;

  if(noFiles == 0) return TRUE;

  // считаем ширину столбца для отображения trdos'ного имени
  PanelInfo panel;
  startupInfo.Control(this, FCTL_GETPANELINFO, &panel);
  int nameColumnWidth = 12;

  if(panel.ViewMode == 4)
  {
    // 25 - ширина остальных колонок,  2 - рамка панели
    nameColumnWidth = panel.PanelRect.right - panel.PanelRect.left - 2 - 25;
  }
  if(nameColumnWidth < 12) nameColumnWidth = 12;
  
  char* name = new char[nameColumnWidth+1];
  
  for(int fNum = 0; fNum < noFiles; ++fNum)
  {
    lstrcpy(item[fNum].FindData.cFileName, pcFiles[fNum].name);
    item[fNum].FindData.nFileSizeLow = files[fNum].size;
    
    item[fNum].CustomColumnNumber = 7;
    item[fNum].CustomColumnData = new LPSTR[item[fNum].CustomColumnNumber];
    
    item[fNum].CustomColumnData[0] = new char[nameColumnWidth+1];
    
    FillMemory(name, nameColumnWidth+1, ' ');
    
    makeTrDosName(name, files[fNum], nameColumnWidth);
    lstrcpy(item[fNum].CustomColumnData[0], name);
    
    item[fNum].CustomColumnData[1] = new char[6];
    wsprintf(item[fNum].CustomColumnData[1], "%5d", files[fNum].start);

    item[fNum].CustomColumnData[2] = new char[4];
    wsprintf(item[fNum].CustomColumnData[2],
             "%3d",
             files[fNum].noSecs);

    item[fNum].CustomColumnData[3] = new char[6];
    wsprintf(item[fNum].CustomColumnData[3], "%5d", files[fNum].packedSize);

    char* method = new char[4];
    switch(files[fNum].pMethod)
    {
      case 0:
                lstrcpy(method, "Str");
                break;
      case 1:
                lstrcpy(method, "LZP");
                break;
      case 2:
                lstrcpy(method, "Shr");
                break;
      case 3:
                if(files[fNum].flags & 0x01)
                 lstrcpy(method, "Imp");
                else
                 lstrcpy(method, "imp");
                break;
      default:
                lstrcpy(method, "???");
    }
    item[fNum].CustomColumnData[4] = method; 

    char* description = detector->description(pcFiles[fNum].type);
    if(description)
    {
      item[fNum].CustomColumnData[5] = new char[lstrlen(description)+1];
      lstrcpy(item[fNum].CustomColumnData[5], description);
    }
    else
    {
      item[fNum].CustomColumnData[5] = new char[1];
      item[fNum].CustomColumnData[5][0] = 0;
    }
    
    char* comment = pcFiles[fNum].comment;
    if(comment)
    {
      item[fNum].CustomColumnData[6] = new char[lstrlen(comment)+1];
      lstrcpy(item[fNum].CustomColumnData[6], comment);
    }
    else
    {
      item[fNum].CustomColumnData[6] = new char[1];
      item[fNum].CustomColumnData[6][0] = 0;
    }
  }
  delete[] name;
  return TRUE;
}

void Manager::freeFindData(PluginPanelItem *panelItem, int noItems)
{
  for (int i = 0; i < noItems; i++)
  {
    delete[] panelItem[i].CustomColumnData[0];
    delete[] panelItem[i].CustomColumnData[1];
    delete[] panelItem[i].CustomColumnData[2];
    delete[] panelItem[i].CustomColumnData[3];
    delete[] panelItem[i].CustomColumnData[4];
    delete[] panelItem[i].CustomColumnData[5];
    delete[] panelItem[i].CustomColumnData[6];
    delete[] panelItem[i].CustomColumnData;
  }
  delete[] panelItem;
}

bool Manager::readInfo(void)
{
  // проверяем не изменился ли файл на диске
  WIN32_FIND_DATA data;
  HANDLE h = FindFirstFile(hostFileName, &data);
  if(h == INVALID_HANDLE_VALUE) return false;
  FindClose(h);
  
  if(CompareFileTime(&data.ftLastWriteTime, &lastModifed.ftLastWriteTime) != 0 ||
     data.nFileSizeLow != lastModifed.nFileSizeLow)
  {
    lastModifed = data;
    if(!openHostFile()) return false;

    noFiles = 0;
    DWORD offset = 0;
    for(;;)
    {
      ZipedFileHdr zHdr;
      if(!setPointer(offset))
      {
        errorMessage(getMsg(MEndOfArchive));
        break;
      }

      if(!read((BYTE*)&zHdr, sizeof(ZipedFileHdr)))
        break;

      if(!isValidZippedFile(zHdr)) break;
      pcFiles[noFiles].offset = offset + sizeof(ZipedFileHdr);
      offset += sizeof(ZipedFileHdr) + zHdr.packedSize;
      CopyMemory(&files[noFiles++], &zHdr, sizeof(ZipedFileHdr));
    }
    makePCNames();
    closeHostFile();
  }
  return true;
}
