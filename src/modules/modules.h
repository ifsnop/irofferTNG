#ifdef _MOD_
#include "../defines.h"
#endif

typedef void (*Function) (void);

#ifndef _MOD_
Function global_func[20];
#endif

#ifdef _MOD_
#define ioutput ((void (*) (int, const char *, ...))f[1])
#define server_write ((void (*) (char *, int))f[2])
#define context_update ((void (*) (const char *, const char *, int))f[3])
#define mycalloc2 ((void * (*) (int, const char *, const char *, int))f[4])
#define mydelete2 ((void (*) (void *))f[5])
#define getpart2 ((char * (*) (const char *, int, const char *, const char *, int))f[6])
#define db_read ((struct db_row_t (*) (const char *, ...))f[7])
#define db_write ((int (*) (const char *, ...))f[8])
#define db_row_clean ((void (*) (struct db_row_t))f[9])
#define server_notice ((void (*) (const char *, const char *, ...))f[10])
#define server_ctcp ((void (*) (const char *, const char *, ...))f[11])
#define stripnonprintable ((char * (*) (char *))f[12])
#endif

