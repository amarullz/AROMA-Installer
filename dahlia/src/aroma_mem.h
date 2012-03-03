//
// MALLOC WRAPPER
//
#ifndef __AROMA_MEM_H__
#define __AROMA_MEM_H__

void *aroma_malloc(size_t size);
void aroma_memory_parentpid(int parent_pid);
void aroma_memory_terminate(const char * message);
#ifndef malloc
  #define malloc(x) aroma_malloc(x)
  #ifndef _AROMA_NODEBUG
    void aroma_free(void * x);
    #define free(x) aroma_free(x)
  #endif
#endif
#ifndef _AROMA_NODEBUG
  void aroma_memory_debug_init();
  void aroma_dump_malloc();
#endif

#endif