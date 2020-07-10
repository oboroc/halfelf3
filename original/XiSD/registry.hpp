#ifndef registry_hpp
#define registry_hpp
#include <windows.h>
#include "types.hpp"

class Registry
{
  public:
    Registry(char* rootKeyName);
    void setNumber(HKEY root, char* valueName, u32   value);
    void setString(HKEY root, char* valueName, char* value);
    
    u32  getNumber(HKEY root, char* valueName, u32   defValue);
    bool getString(HKEY root, char* valueName, char* value, char* defValue, u32 size);
  private:
    HKEY createKey(HKEY root);
    HKEY openKey  (HKEY root);

    char pluginRootKeyName[300];
};

#endif
