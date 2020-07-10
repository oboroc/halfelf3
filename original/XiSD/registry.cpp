#include <windows.h>

#include "types.hpp"
#include "registry.hpp"


Registry::Registry(char* rootKeyName)
{
  lstrcpy(pluginRootKeyName, rootKeyName);
  lstrcat(pluginRootKeyName, "\\ZX\\XiSD\\");
}

HKEY Registry::createKey(HKEY root)
{
  HKEY key;
  u32  actionPerformed;
  RegCreateKeyEx(root,
                 pluginRootKeyName,
                 0,
                 NULL,
                 0,
                 KEY_WRITE,
                 NULL,
                 &key,
                 &actionPerformed);
 return key;
}

HKEY Registry::openKey(HKEY root)
{
  HKEY key;
  if(RegOpenKeyEx(root,pluginRootKeyName,0,KEY_QUERY_VALUE,&key)!=ERROR_SUCCESS)
    return (NULL);
  else
    return key;
}

void Registry::setNumber(HKEY root, char* valueName, u32  value)
{
  HKEY key = createKey(root);
  RegSetValueEx(key,
                valueName,
                0,
                REG_DWORD,
                (u8*)&value,
                sizeof(value));
  RegCloseKey(key);
}

u32 Registry::getNumber(HKEY root, char* valueName, u32 defValue)
{
  HKEY key = openKey(root);
  
  u32 type, value, size = sizeof(defValue);
  int exitCode = RegQueryValueEx(key, valueName, 0, &type, (u8*)&value, &size);
  RegCloseKey(key);
  
  if(key == NULL || exitCode != ERROR_SUCCESS || type != REG_DWORD)
    return defValue;
  else
    return value;
}

void Registry::setString(HKEY root, char* valueName, char* value)
{
  HKEY key = createKey(root);
  RegSetValueEx(key, valueName, 0, REG_SZ, value, lstrlen(value)+1);
  RegCloseKey(key);
}

bool Registry::getString(HKEY root, char* valueName, char* value, char* defValue, u32 size)
{
  HKEY key = openKey(root);
  u32 type;
  int exitCode = RegQueryValueEx(key, valueName, 0, &type, value, &size);
  RegCloseKey(key);

  if(key == NULL || exitCode != ERROR_SUCCESS || type != REG_SZ)
  {
    lstrcpy(value, defValue);
    return false;
  }
  else
    return true;
}
