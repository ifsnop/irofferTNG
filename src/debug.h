
void context_update(const char *file, const char *func, int line);
void context_dump(int dest);
void memory_dump(int dest);
void* mycalloc2(int a, const char *src_function, const char *src_file, int src_line);
void mydelete2(void *t);
char onlyprintable(char a);

#if DEBUG_ENABLED
/*static*/ void meminfo_grow(int grow);
/*static*/ unsigned long mycalloc_hash(void *ptr);
#endif
