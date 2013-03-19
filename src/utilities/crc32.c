#include "crc32.h"

static unsigned long crc_table[256];

void gen_table(void)                /* build the crc table */
{
    unsigned long crc, poly;
    int	i, j;

    poly = 0xEDB88320L;
    for (i = 0; i < 256; i++)
        {
        crc = i;
        for (j = 8; j > 0; j--)
            {
            if (crc & 1)
                crc = (crc >> 1) ^ poly;
            else
                crc >>= 1;
            }
        crc_table[i] = crc;
        }
}


unsigned long get_crc( FILE *fp )    /* calculate the crc value */
{
    register unsigned long crc;
    int c;

    crc = 0xFFFFFFFF;
    while ((c = getc(fp)) != EOF)
        crc = ((crc>>8) & 0x00FFFFFF) ^ crc_table[ (crc^c) & 0xFF ];

    return( crc^0xFFFFFFFF );
}
