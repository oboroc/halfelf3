#include "memory.hpp"

void *malloc(size_t size)
{
  return HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, size);
}

void free(void *block)
{
  HeapFree(GetProcessHeap(),0,block);
}
