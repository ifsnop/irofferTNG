               :#include "defines.h"
               :
  1926  2.2076 :void updatecontext(const char *file, const char *func, int line) { /* updatecontext total:  65860 75.4903 */
               :
               :#if DEBUG_ENABLED
               :        g.context_cur_ptr++;
  1056  1.2104 :        if (g.context_cur_ptr > (2 * DEBUG_MAX_CONTEXTS))
   166  0.1903 :    	    g.context_cur_ptr = g.context_cur_ptr % DEBUG_MAX_CONTEXTS;
 20731 23.7624 :    	g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].file = file;
 21454 24.5911 :    	g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].func = func;
 20144 23.0895 :	g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].line = line;
               :#endif
               :/*    printf ("Trace %3i  %-20s %-16s:%5i\n",
               :    g.context_cur_ptr % DEBUG_MAX_CONTEXTS,
               :        g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].func ?
               :        g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].func : "UNKNOWN",
               :        g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].file ?
               :        g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].file : "UNKNOWN",
               :        g.context_log[g.context_cur_ptr % DEBUG_MAX_CONTEXTS].line);
               :*/						
               :    return;
   383  0.4390 :}
               :
               :/*static*/ unsigned long mycalloc_hash(void *ptr) {
               :unsigned long retval;
               :    
               :    retval = 0xAA;
               :    retval ^= ((unsigned long)ptr >>  0) & 0xFF;
               :    retval ^= ((unsigned long)ptr >>  8) & 0xFF;
               :    retval ^= ((unsigned long)ptr >> 16) & 0xFF;
               :    retval ^= ((unsigned long)ptr >> 24) & 0xFF;
               :
               :    return retval;
               :}
               :			    
               :void* mycalloc2(int a, const char *src_function, const char *src_file, int src_line) {
               :void *t = NULL;
               :int i;
               :#if DEBUG_ENABLED
               :unsigned long start;
               :#endif
               :    
               :    context();
               :
               :    for (i=0; (i<100 && t == NULL); i++)
               :	t = calloc(a,1);
               :		    
               :    if (t == NULL)
               :	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_CRITICAL, "couldn't alloc mem after 100 attempts!!");
               :					
               :#if DEBUG_ENABLED
               :    start = mycalloc_hash(t) * DEBUG_MAX_MEMINFO / 256;
               :
               :    for (i=0; (i<DEBUG_MAX_MEMINFO && g.meminfo[(i+start)%DEBUG_MAX_MEMINFO].ptr); i++) ;
               :    if (i == DEBUG_MAX_MEMINFO) { 
               :	i--; 
               :	ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN,"out of meminfo slots!"); 
               :    }
               :
               :    i = (i+start)%DEBUG_MAX_MEMINFO;
               :
               :    g.meminfo[i].ptr       = t;
               :    g.meminfo[i].alloctime.tv_sec = g.currenttime.tv_sec;
               :    g.meminfo[i].alloctime.tv_usec = g.currenttime.tv_usec;
               :    g.meminfo[i].size      = a;
               :    g.meminfo[i].src_func  = src_function;
               :    g.meminfo[i].src_file  = src_file;
               :    g.meminfo[i].src_line  = src_line;
               :#endif
               :
               :    return t;
               :}
               :
               :void mydelete2(void *t) {
               :#if DEBUG_ENABLED
               :unsigned char *ut = (unsigned char *)t;
               :int i;
               :unsigned long start;
               :#endif
               :	    
               :    context();
               :	
               :    if (t == NULL) 
               :	return;
               :		    
               :#if DEBUG_ENABLED
               :    start = mycalloc_hash(t) * DEBUG_MAX_MEMINFO / 256;
               :
               :    for (i=0; (i<DEBUG_MAX_MEMINFO && (g.meminfo[(i+start)%DEBUG_MAX_MEMINFO].ptr != t)); i++);
               :			    
               :    if (i == DEBUG_MAX_MEMINFO) {
               :        ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN,"pointer 0x%8.8lX not found in meminfo database while trying to free!!",(long)t);
               :        for(i=0; i<(12*12); i+=12) {
               :            ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN," : %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X %2.2X = \"%c%c%c%c%c%c%c%c%c%c%c%c\"",
               :                    ut[i+0], ut[i+1], ut[i+2], ut[i+3], ut[i+4], ut[i+5], ut[i+6], ut[i+7], ut[i+8], ut[i+9], ut[i+10], ut[i+11],
               :                    onlyprintable(ut[i+0]), onlyprintable(ut[i+1]),
               :                    onlyprintable(ut[i+2]), onlyprintable(ut[i+3]),
               :                    onlyprintable(ut[i+4]), onlyprintable(ut[i+5]),
               :                    onlyprintable(ut[i+6]), onlyprintable(ut[i+7]),
               :                    onlyprintable(ut[i+8]), onlyprintable(ut[i+9]),
               :                    onlyprintable(ut[i+10]), onlyprintable(ut[i+11]));
               :        }
               :        ioutput(OUTPUT_DEST_LOG | OUTPUT_TYPE_WARN,"aborting program! (core file should be generated)");
               :	abort(); /* getting a core file will help greatly */
               :    } else {
               :        free(t);
               :        i = (i+start)%DEBUG_MAX_MEMINFO;
               :        g.meminfo[i].ptr       = NULL;
               :        g.meminfo[i].alloctime.tv_sec = 0;
               :        g.meminfo[i].alloctime.tv_usec = 0;	
               :        g.meminfo[i].size      = 0;
               :        g.meminfo[i].src_func  = NULL;
               :        g.meminfo[i].src_file  = NULL;
               :        g.meminfo[i].src_line  = 0;
               :    }
               :#else
               :    free(t);
               :#endif
               :    return;
               :}
               :
               :char onlyprintable(char a){
               :    if (a >= 0x20 && a <= 0x7E)
               :        return a;
               :    else
               :        return '.';
               :}
               :
/* 
 * Total samples for file : "src/debug.c"
 * 
 *  65860 75.4903
 */


/* 
 * Command line: opannotate --source --output-dir=annotated/ ./rb 
 * 
 * Interpretation of command line:
 * Output annotated source file with samples
 * Output all files
 * 
 * CPU: Athlon, speed 1333 MHz (estimated)
 * Counted CPU_CLK_UNHALTED events (Cycles outside of halt state) with a unit mask of 0x00 (No unit mask) count 100000
 */
