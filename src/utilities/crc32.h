#include <stdio.h>         /* then bring it in         */

/* generate the crc table. Must be called before calculating the crc value */
void gen_table(void);

unsigned long get_crc(FILE *);   /* calculate the crc32 value */

