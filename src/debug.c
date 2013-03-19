#include "defines.h"

void context_update(const char *file, const char *func, int line) {
#if DEBUG_ENABLED
    struct context_t *c;

    g.context_cur_ptr++;
    if (g.context_cur_ptr > (2 * DEBUG_MAX_CONTEXTS))
        g.context_cur_ptr = g.context_cur_ptr % DEBUG_MAX_CONTEXTS;
    c = &g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS];
    c->file = file;
    c->func = func;
    c->line = line;
    timeval_update(&c->tv, g.currenttime);
#endif
    return;
}

void context_dump(int dest) {
#if DEBUG_ENABLED
    int i;
    struct context_t *c;
    
    for (i=0; i < DEBUG_MAX_CONTEXTS; i++) {
	c = &g.context_log[(g.context_cur_ptr + 1 + i) % DEBUG_MAX_CONTEXTS];

        ioutput(dest | OUTPUT_TYPE_DEBUG, "   %04i | %03.3fs | %20s:%03d %20s()",
            i - DEBUG_MAX_CONTEXTS + 1,
	    (((float) g.currenttime.tv_sec) + (((float) g.currenttime.tv_usec)/1000000)) -
	    (((float) c->tv.tv_sec) + (((float) c->tv.tv_usec)/1000000)),
	    c->file ? c->file : "UNKNOWN",
	    c->line,
            c->func ? c->func : "UNKNOWN");
    } 
#endif
    return;
}

void memory_dump(int dest) {
#if DEBUG_ENABLED
    int i;

    for (i = 0; i<(DEBUG_MAX_MEMINFO * g.meminfo_depth); i++)
	if (g.meminfo[i].ptr != NULL)
	    ioutput(dest | OUTPUT_TYPE_DEBUG, "%3i %3i | 0x%8.8lX | %6iB | %03.3fs | %20s:%03d %20s()",
	        i / g.meminfo_depth,
	        i % g.meminfo_depth,
	        (long)g.meminfo[i].ptr,
	        g.meminfo[i].size,
	        (((float) g.currenttime.tv_sec) + (((float) g.currenttime.tv_usec)/1000000)) -
	        (((float) g.meminfo[i].alloctime.tv_sec) + (((float) g.meminfo[i].alloctime.tv_usec)/1000000)),
	        g.meminfo[i].src_file,
	        g.meminfo[i].src_line,
	        g.meminfo[i].src_func);
#endif
    return;
}

/*static*/ unsigned long mycalloc_hash(void *ptr) {
unsigned long retval;
    
    retval = 0xAA;
    retval ^= ((unsigned long)ptr >>  0) & 0xFF;
    retval ^= ((unsigned long)ptr >>  8) & 0xFF;
    retval ^= ((unsigned long)ptr >> 16) & 0xFF;
    retval ^= ((unsigned long)ptr >> 24) & 0xFF;

    return retval & (DEBUG_MAX_MEMINFO-1);
}
			    
void* mycalloc2(int a, const char *src_function, const char *src_file, int src_line) {
void *t = NULL;
int i;
#if DEBUG_ENABLED
unsigned long start;
#endif
    
    context();

    for (i=0; (i<100 && t == NULL); i++)
	t = calloc(a,1);
		    
    if (t == NULL)
	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "couldn't alloc mem after 100 attempts!!");
					
#if DEBUG_ENABLED
    if (g.meminfo_count >= ((DEBUG_MAX_MEMINFO * g.meminfo_depth) / 2)) {
	meminfo_grow(g.meminfo_depth/3 + 1);
    } 

    start = mycalloc_hash(t) * g.meminfo_depth;

    for (i=0; g.meminfo[(i+start)%(DEBUG_MAX_MEMINFO * g.meminfo_depth)].ptr; i++) ;
   
    i = (i+start)%(DEBUG_MAX_MEMINFO * g.meminfo_depth); 

    g.meminfo[i].ptr       = t;
    timeval_update(&g.meminfo[i].alloctime, g.currenttime);
    g.meminfo[i].size      = a;
    g.meminfo[i].src_func  = src_function;
    g.meminfo[i].src_file  = src_file;
    g.meminfo[i].src_line  = src_line;
    g.meminfo_count++;
#endif

    return t;
}

void mydelete2(void *t) {
#if DEBUG_ENABLED
unsigned char *ut = (unsigned char *)t;
int i;
unsigned long start;
#endif
	    
    context();
	
    if (t == NULL) 
	return;
		    
#if DEBUG_ENABLED
    start = mycalloc_hash(t) * g.meminfo_depth;

    for (i=0; (i<(DEBUG_MAX_MEMINFO * g.meminfo_depth) && (g.meminfo[(i+start)%(DEBUG_MAX_MEMINFO * g.meminfo_depth)].ptr != t)); i++) ;
      			    
    if (i == (DEBUG_MAX_MEMINFO * g.meminfo_depth)) {
        ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN,"pointer 0x%8.8lX not found in meminfo database while trying to free!!",(long)t);
        for(i=0; i<(12*12); i+=12) {
            ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN," : %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X = \"%c%c%c%c%c%c%c%c%c%c%c%c\"",
                    ut[i+0], ut[i+1], ut[i+2], ut[i+3], ut[i+4], ut[i+5], ut[i+6], ut[i+7], ut[i+8], ut[i+9], ut[i+10], ut[i+11],
                    onlyprintable(ut[i+0]), onlyprintable(ut[i+1]),
                    onlyprintable(ut[i+2]), onlyprintable(ut[i+3]),
                    onlyprintable(ut[i+4]), onlyprintable(ut[i+5]),
                    onlyprintable(ut[i+6]), onlyprintable(ut[i+7]),
                    onlyprintable(ut[i+8]), onlyprintable(ut[i+9]),
                    onlyprintable(ut[i+10]), onlyprintable(ut[i+11]));
        }
        ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN,"aborting program! (core file should be generated)");
	abort(); /* getting a core file will help greatly */
    } else {
        free(t);
        i = (i+start)%(DEBUG_MAX_MEMINFO * g.meminfo_depth);
        g.meminfo[i].ptr       = NULL;
	timeval_reset(&g.meminfo[i].alloctime);
        g.meminfo[i].size      = 0;
        g.meminfo[i].src_func  = NULL;
        g.meminfo[i].src_file  = NULL;
        g.meminfo[i].src_line  = 0;
	g.meminfo_count--; 
	
	if ((g.meminfo_depth > 1) &&
	    (g.meminfo_count < ((DEBUG_MAX_MEMINFO * g.meminfo_depth) / 8))) {
	    meminfo_grow(-1);
        } 
    }
#else
    free(t);
#endif
    return;
}

char onlyprintable(char a){
    if (a >= 0x20 && a <= 0x7E)
        return a;
    else
        return '.';
}

#if DEBUG_ENABLED
/*static*/ void meminfo_grow(int grow) {
struct meminfo_t *newmeminfo;
int cc, dd, len, i, start;
	    
    ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_INFO, "growing meminfo from %d to %d",
	g.meminfo_depth, g.meminfo_depth+grow);

    len = DEBUG_MAX_MEMINFO * sizeof(struct meminfo_t) * (g.meminfo_depth+grow);
    newmeminfo = calloc(len,1);

    /* replace zero entry */
    if (g.meminfo) {
	g.meminfo[0].ptr       = NULL;
	timeval_reset(&g.meminfo[0].alloctime);
	g.meminfo[0].size      = 0;
        g.meminfo[0].src_func  = NULL;
	g.meminfo[0].src_file  = NULL;
        g.meminfo[0].src_line  = 0;
    } else { /* first time, count item #0 */
	g.meminfo_count++;
    }

    newmeminfo[0].ptr          = newmeminfo;
    timeval_update(&newmeminfo[0].alloctime, g.currenttime);
    newmeminfo[0].size         = len;
    newmeminfo[0].src_func     = __FUNCTION__;
    newmeminfo[0].src_file     = __FILE__;
    newmeminfo[0].src_line     = __LINE__;

    for (cc=0; cc<DEBUG_MAX_MEMINFO; cc++) {
        for (dd=0; dd<g.meminfo_depth; dd++) {
	    if (g.meminfo[(cc*(g.meminfo_depth)) + dd].ptr) {
                /* find new location */
                start = mycalloc_hash(g.meminfo[(cc*(g.meminfo_depth)) + dd].ptr) * (g.meminfo_depth+grow);
                for (i=0; newmeminfo[(i+start)%(DEBUG_MAX_MEMINFO * (g.meminfo_depth+grow))].ptr; i++) ;
                i = (i+start)%(DEBUG_MAX_MEMINFO * (g.meminfo_depth+grow));
        	newmeminfo[i] = g.meminfo[(cc*(g.meminfo_depth)) + dd];
	    }
	}
    }

    if (g.meminfo) {
    /* second or later time */
	free(g.meminfo);
    }
		       
    g.meminfo = newmeminfo;
    g.meminfo_depth += grow;
    return;
}
#endif
