/*
Eugene Kotlyarov <ek@oris.ru>
*/
#ifndef memory_hpp
#define memory_hpp

#include <windows.h>

void* malloc(size_t size);
void  free  (void *block);

#endif
