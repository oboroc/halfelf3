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
    SetFilePointer(hostFile, offset + volumeOffset[vNum], NULL, FILE_BEGIN);
    return true;
  }
  CloseHandle(hostFile);

  hostFile = open(vNum);
  if(hostFile == INVALID_HANDLE_VALUE) return false;
  SetFilePointer(hostFile, offset + volumeOffset[vNum], NULL, FILE_BEGIN);
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
  SetFilePointer(hostFile, volumeOffset[volumeNum], NULL, FILE_BEGIN);
  ReadFile(hostFile, (BYTE*)buf + noBytesRead, size - noBytesRead, &noBytesRead, NULL);

  return true;
}

Manager::Manager(char* fileName, int offset)
{
  lstrcpy(hostFileName, fileName);
  hdrOffset = offset;
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

  volumeSize  [0] = fileSize - hdrOffset;
  volumeName  [0] = new char[300];
  volumeOffset[0] = hdrOffset;
  lstrcpy(volumeName[0], hostFileName);

  char name[300];
  lstrcpy(name, hostFileName);
  char *ptr = name + lstrlen(name);
  while(ptr != name && *ptr != '.') --ptr;
  if(ptr!=name) *(ptr+2) = 0;
  lstrcat(name, "*");
  
  for(noVolumes = 1; noVolumes < 16; noVolumes++)
  {
    bool found = false;

    WIN32_FIND_DATA data;
    HANDLE h = FindFirstFile(name, &data);
    if(h == INVALID_HANDLE_VALUE) break;

    do
    {
      char tmpName[300];
      lstrcpy(tmpName, hostFileName);
      lstrcpy(pointToName(tmpName), data.cFileName);

      HANDLE volume = CreateFile(tmpName,
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
      CloseHandle(volume);

      if(hdr.checkSum != calculateCheckSum(hdr)) continue;

      char type = '0' + noVolumes - 1;
      if(type > '9') type += 'A' - '9' - 1;
      if(hdr.type == type && !memcmp(&hdr.start, "rp", 2))
      {
        found = true;
        volumeSize  [noVolumes] = data.nFileSizeLow - sizeof(HoHdr);
        volumeOffset[noVolumes] = sizeof(HoHdr);
        volumeName  [noVolumes] = new char[300];
        lstrcpy(volumeName[noVolumes], tmpName);
        break;
      }
    }while(FindNextFile(h, &data));

    FindClose(h);
    if(!found) break;
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
  info->Format        = "HRiP";
  info->StartSortMode = SM_UNSORTED;
  
  static char panelTitle[260];
  panelTitle[0] = 0;
  lstrcat(panelTitle, " HRiP:");
  lstrcat(panelTitle, pointToName(hostFileName));
  lstrcat(panelTitle, " ");
  info->PanelTitle = panelTitle;

  static PanelMode   mode[10];
  static char*       columnTitles3[4];
  columnTitles3[0] = getMsg(MName);
  columnTitles3[1] = getMsg(MSectorSize);
  columnTitles3[2] = getMsg(MName);
  columnTitles3[3] = getMsg(MSectorSize);

  static char*       columnTitles4[5];
  columnTitles4[0] = getMsg(MName);
  columnTitles4[1] = getMsg(MSize);
  columnTitles4[2] = getMsg(MStart);
  columnTitles4[3] = getMsg(MSectorSize);
  columnTitles4[4] = getMsg(MPackedSize);
  
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
  // C4 - описание типа
  // C5 - комментарий к файлу
  
  mode[3].ColumnTypes="N,C2,N,C2";
  mode[3].ColumnWidths="0,3,0,3";
  mode[3].ColumnTitles=columnTitles3;
  mode[3].StatusColumnTypes="C0,S,C2";
  mode[3].StatusColumnWidths="0,5,3";
  mode[3].AlignExtensions=TRUE;
  mode[3].FullScreen=FALSE;
  
  mode[4].ColumnTypes="C0,S,C1,C2,C3";
  mode[4].ColumnWidths="0,5,5,3,5";
  mode[4].StatusColumnTypes="N,S,C2";
  mode[4].StatusColumnWidths="0,5,3";
  mode[4].ColumnTitles=columnTitles4;
  mode[4].FullScreen=FALSE;
  
  mode[5].ColumnTypes="C0,C4";
  mode[5].ColumnWidths="12,0";
  mode[5].StatusColumnTypes="N,S,C2";
  mode[5].StatusColumnWidths="0,5,3";
  mode[5].ColumnTitles=columnTitles5;
  mode[5].FullScreen=FALSE;

  mode[6].ColumnTypes="C0,C5";
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
    // 21 - ширина остальных колонок,  2 - рамка панели
    nameColumnWidth = panel.PanelRect.right - panel.PanelRect.left - 2 - 21;
  }
  if(nameColumnWidth < 12) nameColumnWidth = 12;
  
  char* name = new char[nameColumnWidth+1];
  
  for(int fNum = 0; fNum < noFiles; ++fNum)
  {
    lstrcpy(item[fNum].FindData.cFileName, pcFiles[fNum].name);
    item[fNum].FindData.nFileSizeLow = files[fNum].size;
    
    item[fNum].CustomColumnNumber = 6;
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
    wsprintf(item[fNum].CustomColumnData[3], "%5d", infos[fNum].packedSize);

    char* description = detector->description(pcFiles[fNum].type);
    if(description)
    {
      item[fNum].CustomColumnData[4] = new char[lstrlen(description)+1];
      lstrcpy(item[fNum].CustomColumnData[4], description);
    }
    else
    {
      item[fNum].CustomColumnData[4] = new char[1];
      item[fNum].CustomColumnData[4][0] = 0;
    }
    
    char* comment = pcFiles[fNum].comment;
    if(comment)
    {
      item[fNum].CustomColumnData[5] = new char[lstrlen(comment)+1];
      lstrcpy(item[fNum].CustomColumnData[5], comment);
    }
    else
    {
      item[fNum].CustomColumnData[5] = new char[1];
      item[fNum].CustomColumnData[5][0] = 0;
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

    SetFilePointer(hostFile, hdrOffset+3, NULL, FILE_BEGIN);
    DWORD noBytesRead;
    ReadFile(hostFile, &noFiles, 1, &noBytesRead, NULL);
    if(noFiles == 'P') noFiles = 0xFF; // для HRiP версии 0.98

    BlockHdr block;
    ExtInfo  info;
    // перемещаемся на первый блок в архиве

    DWORD offset = 8;
    setPointer(offset);
    bool isUnexpectedEnd = false;
    for(int i = 0; i < noFiles && !isUnexpectedEnd; ++i)
    {
      WORD pSize = 0;
      infos[i].offset = offset;
      do
      {
        ZeroMemory(&block, sizeof(BlockHdr));
        read(&block, 6);
        
        char signature[] = { 'H', 'r', 's', 't', '2' };
        if(memcmp(&block.signature, signature, sizeof(signature)))
        {
          if(noFiles != 0xFF) // для HRiP версии 0.98
          {
            errorMessage(getMsg(MEndOfArchive));
            isUnexpectedEnd = true;
          }
          noFiles = i;
          break;
        }
        
        // каталоги пропускаем
        if(block.flags & 0x80)
        {
          BYTE dirHdrSize, dirHdr[255];
          read(&dirHdrSize, 1);
          read(dirHdr, dirHdrSize);
          offset += 7 + dirHdrSize;
          setPointer(offset);
          infos[i].offset = offset;
          continue;
        }
        
        // читаем остаток заголовка блока
        read(&block.size, 5);
        
        pSize += block.pSize;

        if(block.extInfoSize >= 18)
        {
          read(&info, sizeof(ExtInfo));
          CopyMemory(&files[i], &info.file, sizeof(FileHdr));
        }
        // если длина дополнительной информации нестандартная,
        // пропускаем доп. информацию.
        // !!!! надо подумать о создании файла с каким-нибудь именем
        
        offset += sizeof(BlockHdr) + block.extInfoSize + block.pSize;
        setPointer(offset);
      } while(!(block.flags & 0x02));
      
      infos[i].packedSize = pSize;
    }
    makePCNames();
    closeHostFile();
  }
  return true;
}
