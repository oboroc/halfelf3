#ifndef MANAGER_HPP
#define MANAGER_HPP

#include <windows.h>
#include "plugin.hpp"
#include "types.hpp"

class Manager
{
  public:
    Manager(char* fileName, int offset);
    ~Manager();
    void getOpenPluginInfo(OpenPluginInfo *info);
    
    int  getFindData      (PluginPanelItem **pPanelItem, int *pNoItems, int opMode);
    void freeFindData     (PluginPanelItem *panelItem,   int  noItems);
    int  getFiles         (PluginPanelItem *panelItem, int noItems, int move, char *destPath, int opMode);
  private:
    char            hostFileName[260];
    HANDLE          hostFile;
    int             hdrOffset;
    WIN32_FIND_DATA lastModifed;
    BYTE            noFiles;

    FileHdr         files  [255];
    ExtFileHdr      pcFiles[255];
    PackInfo        infos  [255];

    int   noVolumes;
    DWORD volumeSize  [16];
    WORD  volumeOffset[16];
    char* volumeName  [16];
    int   volumeNum;

    bool openHostFile (void);
    void closeHostFile(void);

    bool readInfo     (void);
    void makePCNames  (void);

    bool extractFile  (int fNum, BYTE* buffer, int size);

    HANDLE open      (int fNum);
    bool   read      (void* buf, WORD size);
    bool   setPointer(DWORD offset);
};

#endif
