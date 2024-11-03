//#define WIN32_LEAN_AND_MEAN
//#define VC_EXTRALEAN

#include "../far-sdk/plugin.hpp"
#include "xTRDLng.hpp"
#include "version.hpp"
#include <initguid.h>
#include "guid.hpp"
#include "xTRD.hpp"

static struct PluginStartupInfo Info;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{
	Info->StructSize=sizeof(struct GlobalInfo);
	Info->MinFarVersion=FARMANAGERVERSION;
	Info->Version=PLUGIN_VERSION;
	Info->Guid=MainGuid;
	Info->Title=PLUGIN_NAME;
	Info->Description=PLUGIN_DESC;
	Info->Author=PLUGIN_AUTHOR;
}

/*
 Function GetMsg returns a message string from the language file.
 It is a layer on top of Info.GetMsg for shorter code :-)
*/
const wchar_t *GetMsg(int MsgId)
{
	return Info.GetMsg(&MainGuid,MsgId);
}

/*
 Function SetStartupInfoW is called once, before any other functions.
 It provides the plugin information it needs to work.
*/
void WINAPI SetStartupInfoW(const struct PluginStartupInfo *psi)
{
	Info=*psi;
/*  reg                    = new Registry(startupInfo.RootKey);
  
  op.reread              = false;
  op.showExt             = reg->getNumber(HKEY_CURRENT_USER, "ShowExt",            1);
  op.defaultPanelMode    = reg->getNumber(HKEY_CURRENT_USER, "DefaultPanelMode",   5);
  op.defaultFormat       = reg->getNumber(HKEY_CURRENT_USER, "DefaultFormat",      1);
  op.detectFormat        = reg->getNumber(HKEY_CURRENT_USER, "DetectFormat",       1);
  op.autoMove            = reg->getNumber(HKEY_CURRENT_USER, "AutoMove",           1);
  op.useDS               = reg->getNumber(HKEY_CURRENT_USER, "UseDS",              1);

  char cmdLine1[300] = "c:\\dir1\\emul1.exe %P [%N]";
  char cmdLine2[300] = "c:\\dir2\\emul2.exe %d \"%f\"";
  char types1[100] = "Bb";
  char types2[100] = "*";
  reg->getString(HKEY_CURRENT_USER, "CmdLine1", op.cmdLine1, cmdLine1, 300);
  reg->getString(HKEY_CURRENT_USER, "CmdLine2", op.cmdLine2, cmdLine2, 300);
  reg->getString(HKEY_CURRENT_USER, "Types1",   op.types1,   types1,   100);
  reg->getString(HKEY_CURRENT_USER, "Types2",   op.types2,   types2,   100);
  op.fullScreen1 = reg->getNumber(HKEY_CURRENT_USER, "FullScreen1", 0);
  op.fullScreen2 = reg->getNumber(HKEY_CURRENT_USER, "FullScreen2", 0);

  char iniFilePath[300] = "";
  reg->getString(HKEY_CURRENT_USER, "IniFilePath", op.iniFilePath, iniFilePath, 300);
  
  detector            = new Detector (startupInfo.ModuleName);
  fmtReader           = new FmtReader(startupInfo.ModuleName);*/
}


void WINAPI ExitFARW()
{
/*  delete reg;
  delete detector;
  delete fmtReader;*/
}


/*
 Function GetPluginInfoW is called to get information about the plugin
*/
void WINAPI GetPluginInfoW(struct PluginInfo *Info)
{
	Info->StructSize=sizeof(*Info);
	Info->Flags=PF_EDITOR;
	static const wchar_t *PluginMenuStrings[1];
	PluginMenuStrings[0]=GetMsg(MTitle);
	Info->PluginMenu.Guids=&MenuGuid;
	Info->PluginMenu.Strings=PluginMenuStrings;
	Info->PluginMenu.Count=ARRAYSIZE(PluginMenuStrings);
}

/*
 Function OpenW is called each time a new plugin copy is created.
*/
HANDLE WINAPI OpenW(const struct OpenInfo *OInfo)
{
	const wchar_t *MsgItems[]=
	{
		GetMsg(MTitle),
		GetMsg(MMessage1),
		GetMsg(MMessage2),
		GetMsg(MMessage3),
		GetMsg(MMessage4),
		L"\x01",                      /* separator line */
		GetMsg(MButton),
	};

	Info.Message(&MainGuid,           /* GUID */
		NULL,
		FMSG_WARNING|FMSG_LEFTALIGN,  /* Flags */
		L"Contents",                  /* HelpTopic */
		MsgItems,                     /* Items */
		ARRAYSIZE(MsgItems),          /* ItemsNumber */
		1);                           /* ButtonsNumber */

	return NULL;
}
